/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "VolumeClouds.h"
#include "CloudNoiseTextureGenerator.h"
#include "SkyboltVis/Camera.h"
#include "SkyboltVis/OsgGeometryFactory.h"
#include "SkyboltVis/OsgImageHelpers.h"
#include "SkyboltVis/OsgStateSetHelpers.h"
#include "SkyboltVis/OsgTextureHelpers.h"
#include "SkyboltVis/RenderContext.h"

#include <osg/CullFace>
#include <osg/Geode>
#include <osg/ShapeDrawable>
#include <osg/Texture2D>
#include <osg/Texture3D>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

//#define WIREFRAME
#ifdef WIREFRAME
#include <osg/PolygonMode>
#endif

using namespace skybolt::vis;

static osg::Texture3D* createTexture3D(const osg::ref_ptr<osg::Image> image)
{
	osg::Texture3D* texture = new osg::Texture3D(image);
	texture->setFilter(osg::Texture::FilterParameter::MIN_FILTER, osg::Texture::FilterMode::LINEAR_MIPMAP_LINEAR);
	texture->setFilter(osg::Texture::FilterParameter::MAG_FILTER, osg::Texture::FilterMode::LINEAR);
	texture->setWrap(osg::Texture::WRAP_R, osg::Texture::REPEAT);
	texture->setWrap(osg::Texture::WRAP_S, osg::Texture::REPEAT);
	texture->setWrap(osg::Texture::WRAP_T, osg::Texture::REPEAT);
	return texture;
}

static osg::StateSet* createStateSet(const osg::ref_ptr<osg::Program>& program, const VolumeClouds::Uniforms& uniforms, const osg::ref_ptr<osg::Texture2D>& cloudsTexture)
{
	assert(program);
	assert(cloudsTexture);

	osg::StateSet* stateSet = new osg::StateSet();
	stateSet->setAttributeAndModes(program, osg::StateAttribute::ON);

	stateSet->setMode(GL_CULL_FACE, osg::StateAttribute::OFF);
	stateSet->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);

	stateSet->addUniform(uniforms.modelMatrixUniform);
	stateSet->addUniform(uniforms.topLeftDir);
	stateSet->addUniform(uniforms.topRightDir);
	stateSet->addUniform(uniforms.bottomLeftDir);
	stateSet->addUniform(uniforms.bottomRightDir);
	stateSet->addUniform(uniforms.jitterOffset);

#ifdef WIREFRAME
	osg::PolygonMode * polygonMode = new osg::PolygonMode;
	polygonMode->setMode(osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::LINE);

	stateSet->setAttributeAndModes(polygonMode, osg::StateAttribute::OVERRIDE | osg::StateAttribute::ON);
#endif

	int unit = 0;
	{
		stateSet->setTextureAttributeAndModes(unit, cloudsTexture);
		stateSet->addUniform(createUniformSampler2d("globalAlphaSampler", unit++));
	}
	{
		osg::ref_ptr<osg::Texture> texture = new osg::Texture2D(osgDB::readImageFile("Environment/Noise.png"));
		texture->setWrap(osg::Texture::WRAP_S, osg::Texture::REPEAT);
		texture->setWrap(osg::Texture::WRAP_T, osg::Texture::REPEAT);
		stateSet->setTextureAttributeAndModes(unit, texture);
		stateSet->addUniform(createUniformSampler2d("coverageDetailSampler", unit++));
	}

	{
//#define GENERATE_VOLUME_TEXTURE
//#define CONVERT_VOLUME_TEXTURE_FROM_LAYER_IMAGES
#ifdef GENERATE_VOLUME_TEXTURE
		{
			PerlinWorleyConfig config;
			config.width = 128;
			config.perlinOctaves = 6;
			config.perlinFrequency = 8.0f;
			config.worley.invert = true;

			osg::ref_ptr<osg::Image> image = createPerlinWorleyTexture3d(config);
			writeTexture3d(*image, "Assets/Core/Environment/Cloud/CloudVolumeBase.png");
		}
#else
#ifdef CONVERT_VOLUME_TEXTURE_FROM_LAYER_IMAGES
	osg::ref_ptr<osg::Image> image = readTexture3dFromSeparateFiles("D:/Programming/MyProjects/Skybolt/AssetsSource/Clouds/my3DTextureArray.", ".tga", 128);
	writeTexture3d(*image, "Assets/Core/Environment/Cloud/CloudVolumeBase.png");
#else
		osg::ref_ptr<osg::Image> image = readTexture3d("Environment/Cloud/CloudVolumeBase.png");
#endif
#endif

//#define GENERATE_COVERAGE_NOISE_TEXTURE
#ifdef GENERATE_COVERAGE_NOISE_TEXTURE
		{
			PerlinWorleyConfig config;
			config.width = 512;
			config.perlinOctaves = 8;
			config.perlinFrequency = 4.0f;
			config.worley.octaveCount = 4;
			config.worley.frequency = 16.0f;
			config.worley.lacunarity = 2.0f;
			config.worley.amplitude = 0.15;
			config.worley.gain = 0.9;
			config.worley.invert = true;

			osg::ref_ptr<osg::Image> image = createPerlinWorleyTexture2d(config);
			normalize(*image);
			osgDB::writeImageFile(*image, "CloudNoise.png");
		}
#endif

		static osg::ref_ptr<osg::Texture3D> texture = createTexture3D(image);

		stateSet->setTextureAttributeAndModes(unit, texture);
		stateSet->addUniform(createUniformSampler3d("noiseVolumeSampler", unit++));
	}
	stateSet->addUniform(uniforms.upscaleFactor);

	return stateSet;
}

VolumeClouds::VolumeClouds(const VolumeCloudsConfig& config) :
	mApplyTemporalUpscalingJitter(config.applyTemporalUpscalingJitter)
{
	mGeode = new osg::Geode();
	mGeode->setCullingActive(false);

	mUniforms.modelMatrixUniform = new osg::Uniform("modelMatrix", osg::Matrixf());
	mUniforms.topLeftDir = new osg::Uniform("topLeftDir", osg::Vec3f(0, 0, 0));
	mUniforms.topRightDir = new osg::Uniform("topRightDir", osg::Vec3f(0, 0, 0));
	mUniforms.bottomLeftDir = new osg::Uniform("bottomLeftDir", osg::Vec3f(0, 0, 0));
	mUniforms.bottomRightDir = new osg::Uniform("bottomRightDir", osg::Vec3f(0, 0, 0));

	// Specifies how much we need to increase texture LOD to account for upscaling.
	// Although the upscaling factor is 4x we use a factor of 3x here to prevent excessive jittering.
	mUniforms.upscaleFactor = new osg::Uniform("upscaleTextureLodFactor", mApplyTemporalUpscalingJitter ? 3.f : 1.f);

	mUniforms.jitterOffset = new osg::Uniform("jitterOffset", osg::Vec2f(0.f, 0.f));

	osg::StateSet* stateSet = createStateSet(config.program, mUniforms, config.cloudsTexture);

	osg::Vec2f pos(0,0);
	osg::Vec2f size(1,1);

	static osg::ref_ptr<osg::Geometry> quad = createQuadWithUvs(BoundingBox2f(pos, size), QuadUpDirectionY);
	quad->setCullingActive(false);
	quad->setStateSet(stateSet);
	mTransform->addChild(quad);
}

VolumeClouds::~VolumeClouds()
{
}

// From https://en.wikipedia.org/wiki/Ordered_dithering
const int bayerIndices[] = {
    0, 8, 2, 10,
    12, 4, 14, 6,
    3, 11, 1, 9,
	15, 7, 13, 5
};

const std::vector<osg::Vec2f> calcBayerOffsets(const int* bayerIndices)
{
	std::vector<osg::Vec2f> r;
	for (int i = 0; i < 16; ++i)
	{
		for (int j = 0; j < 16; ++j)
		{
			if (bayerIndices[j] == i)
			{
				r.push_back(osg::Vec2f((float(j % 4) + 0.5f) / 4.0f, (float(j / 4) + 0.5f) / 4.0f));
				break;
			}
		}
	}
	return r;
};

void VolumeClouds::updatePreRender(const CameraRenderContext& context)
{
	osg::Matrixf modelMatrix = mTransform->getWorldMatrices().front();
	mUniforms.modelMatrixUniform->set(modelMatrix);

	const Camera& camera = context.camera;
	// Update uniforms
	{
		osg::Vec3f cameraPosition = camera.getPosition();

		osg::Matrix proj = camera.getProjectionMatrix();


		if (mApplyTemporalUpscalingJitter)
		{
			int frameNumberMod = context.frameNumber % 16;

			static std::vector<osg::Vec2f> bayerOffset = calcBayerOffsets(bayerIndices);

			mJitterOffset = osg::Vec2(
				(bayerOffset[(frameNumberMod)].x() - 0.5f) / float(context.targetDimensions.x()) * 4.f,
				(bayerOffset[(frameNumberMod)].y() - 0.5f) / float(context.targetDimensions.y()) * 4.f);
			proj(2, 0) += mJitterOffset.x() * 2.f;
			proj(2, 1) += mJitterOffset.y() * 2.f;
		}

		mUniforms.jitterOffset->set(mJitterOffset);

		osg::Matrixf viewProj = camera.getViewMatrix() * proj;
		osg::Matrixf viewProjInv = osg::Matrix::inverse(viewProj);

		osg::Vec4f c00 = osg::Vec4f(-1, -1, 0.5f, 1.0f) * viewProjInv;
		c00 /= c00.w();
		osg::Vec4f c10 = osg::Vec4f(1, -1, 0.5f, 1.0f) * viewProjInv;
		c10 /= c10.w();
		osg::Vec4f c01 = osg::Vec4f(-1, 1, 0.5f, 1.0f) * viewProjInv;
		c01 /= c01.w();
		osg::Vec4f c11 = osg::Vec4f(1, 1, 0.5f, 1.0f) * viewProjInv;
		c11 /= c11.w();

		osg::Vec3f dir00 = osg::Vec3f(c00.x(), c00.y(), c00.z()) - cameraPosition;
		dir00.normalize();
		osg::Vec3f dir10 = osg::Vec3f(c10.x(), c10.y(), c10.z()) - cameraPosition;
		dir10.normalize();
		osg::Vec3f dir01 = osg::Vec3f(c01.x(), c01.y(), c01.z()) - cameraPosition;
		dir01.normalize();
		osg::Vec3f dir11 = osg::Vec3f(c11.x(), c11.y(), c11.z()) - cameraPosition;
		dir11.normalize();

		mUniforms.topLeftDir->set(dir00);
		mUniforms.topRightDir->set(dir10);
		mUniforms.bottomLeftDir->set(dir01);
		mUniforms.bottomRightDir->set(dir11);
	}
}

osg::Matrix VolumeClouds::getModelMatrix() const
{
	return mTransform->getWorldMatrices().front();
}