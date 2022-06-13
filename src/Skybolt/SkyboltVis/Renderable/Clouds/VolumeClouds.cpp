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
#include "SkyboltVis/TextureGenerator/TextureGeneratorCameraFactory.h"

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

	return stateSet;
}

static osg::ref_ptr<osg::Texture2D> createCloudColorTexture(int width, int height)
{
	osg::ref_ptr<osg::Texture2D> texture = createRenderTexture(width, height);
	texture->setResizeNonPowerOfTwoHint(false);
	texture->setInternalFormat(GL_RGBA);
	texture->setFilter(osg::Texture2D::FilterParameter::MIN_FILTER, osg::Texture2D::FilterMode::LINEAR);
	texture->setFilter(osg::Texture2D::FilterParameter::MAG_FILTER, osg::Texture2D::FilterMode::LINEAR);
	texture->setNumMipmapLevels(0);
	texture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
	texture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);

	return texture;
}

static osg::ref_ptr<osg::Texture2D> createCloudDepthTexture(int width, int height)
{
	osg::ref_ptr<osg::Texture2D> texture = createRenderTexture(width, height);
	texture->setResizeNonPowerOfTwoHint(false);
	texture->setInternalFormat(GL_R32F);
	texture->setFilter(osg::Texture2D::FilterParameter::MIN_FILTER, osg::Texture2D::FilterMode::LINEAR);
	texture->setFilter(osg::Texture2D::FilterParameter::MAG_FILTER, osg::Texture2D::FilterMode::LINEAR);
	texture->setNumMipmapLevels(0);
	texture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
	texture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);

	return texture;
}

static osg::StateSet* createTexturedQuadStateSet(osg::ref_ptr<osg::Program> program, osg::ref_ptr<osg::Texture2D> colorTexture, osg::ref_ptr<osg::Texture2D> depthTexture)
{
	osg::StateSet* stateSet = new osg::StateSet();
	stateSet->setAttributeAndModes(program, osg::StateAttribute::ON);
	stateSet->setTextureAttributeAndModes(0, colorTexture, osg::StateAttribute::ON);
	stateSet->setTextureAttributeAndModes(1, depthTexture, osg::StateAttribute::ON);
	stateSet->addUniform(createUniformSampler2d("colorTexture", 0));
	stateSet->addUniform(createUniformSampler2d("depthTexture", 1));
	stateSet->setMode(GL_CULL_FACE, osg::StateAttribute::OFF);
	return stateSet;
}

VolumeClouds::VolumeClouds(const VolumeCloudsConfig& config)
{
	mGeode = new osg::Geode();
	mGeode->setCullingActive(false);

	mUniforms.modelMatrixUniform = new osg::Uniform("modelMatrix", osg::Matrixf());
	mUniforms.topLeftDir = new osg::Uniform("topLeftDir", osg::Vec3f(0, 0, 0));
	mUniforms.topRightDir = new osg::Uniform("topRightDir", osg::Vec3f(0, 0, 0));
	mUniforms.bottomLeftDir = new osg::Uniform("bottomLeftDir", osg::Vec3f(0, 0, 0));
	mUniforms.bottomRightDir = new osg::Uniform("bottomRightDir", osg::Vec3f(0, 0, 0));

	osg::StateSet* stateSet = createStateSet(config.program, mUniforms, config.cloudsTexture);

	osg::Vec2f pos(0,0);
	osg::Vec2f size(1,1);

	static osg::ref_ptr<osg::Geometry> quad = createQuadWithUvs(BoundingBox2f(pos, size), QuadUpDirectionY);
	quad->setCullingActive(false);

#define COMPOSITE_CLOUDS
#ifdef COMPOSITE_CLOUDS
	// TODO: fit to window dimensions
	int width = 512;
	int height = 256;
	mColorTexture = createCloudColorTexture(width, height);
	osg::ref_ptr<osg::Texture2D> depthTexture = createCloudDepthTexture(width, height);
	
	TextureGeneratorCameraFactory factory;
	osg::ref_ptr<osg::Camera> camera = factory.createCameraWithQuad({ mColorTexture, depthTexture }, stateSet, /* clear */ false);
	mTransform->addChild(camera);

	osg::StateSet* texturedQuadStateSet = createTexturedQuadStateSet(config.compositorProgram, mColorTexture, depthTexture);
	makeStateSetTransparent(*texturedQuadStateSet, vis::TransparencyMode::PremultipliedAlpha, RenderBinId::Clouds);

	mGeode->setStateSet(texturedQuadStateSet);
#else
	makeStateSetTransparent(*stateSet, vis::TransparencyMode::PremultipliedAlpha);
	mGeode->setStateSet(stateSet);
#endif
	mGeode->addDrawable(quad);
	mTransform->addChild(mGeode);
}

VolumeClouds::~VolumeClouds()
{
	mTransform->removeChild(mGeode);
}

void VolumeClouds::updatePreRender(const RenderContext& context)
{
	osg::Matrixf modelMatrix = mTransform->getWorldMatrices().front();
	mUniforms.modelMatrixUniform->set(modelMatrix);

	const Camera& camera = context.camera;
	// Update uniforms
	{
		osg::Vec3f cameraPosition = camera.getPosition();

		osg::Matrixf viewProj = camera.getViewMatrix() * camera.getProjectionMatrix();
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
