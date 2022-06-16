/* Copyright 2012-2021 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "ShadowMapGenerator.h"
#include "SkyboltVis/MatrixHelpers.h"
#include "SkyboltVis/OsgTextureHelpers.h"
#include "SkyboltVis/VisibilityCategory.h"

#include <osg/Camera>

#include <functional>

namespace skybolt {
namespace vis {

static void getOrthonormalBasis(const osg::Vec3 &normal, osg::Vec3 &tangent, osg::Vec3 &bitangent)
{
	float d = normal * osg::Vec3(0, 0, -1);
	if (d > -0.95f && d < 0.95f)
		bitangent = normal ^ osg::Vec3(0, 0, -1);
	else
		bitangent = normal ^ osg::Vec3(0, 1, 0);
	bitangent.normalize();
	tangent = bitangent ^ normal;
}

ShadowMapGenerator::ShadowMapGenerator(const ShadowMapGeneratorConfig& config)
{
	int textureWidth = config.textureSize;
	int textureHeight = config.textureSize;

	mTexture = createRenderTexture(textureWidth, textureHeight);

	mTexture->setResizeNonPowerOfTwoHint(false);
	mTexture->setInternalFormat(GL_DEPTH_COMPONENT);
	mTexture->setShadowComparison(true);
	mTexture->setShadowTextureMode(osg::Texture2D::LUMINANCE);
	mTexture->setTextureWidth(textureWidth);
	mTexture->setTextureHeight(textureHeight);
	mTexture->setFilter(osg::Texture2D::FilterParameter::MIN_FILTER, osg::Texture2D::FilterMode::LINEAR);
	mTexture->setFilter(osg::Texture2D::FilterParameter::MAG_FILTER, osg::Texture2D::FilterMode::LINEAR);
	mTexture->setNumMipmapLevels(0);
	mTexture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
	mTexture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);

	mCamera = new osg::Camera;
	mCamera->setClearDepth(1.0);
	mCamera->setClearMask(GL_DEPTH_BUFFER_BIT);
	mCamera->setComputeNearFarMode(osg::Camera::DO_NOT_COMPUTE_NEAR_FAR);
	mCamera->setViewport(0, 0, mTexture->getTextureWidth(), mTexture->getTextureHeight());
	mCamera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
	mCamera->setRenderOrder(osg::Camera::PRE_RENDER);
	mCamera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);
	mCamera->detach(osg::Camera::COLOR_BUFFER);
	mCamera->attach(osg::Camera::DEPTH_BUFFER, mTexture);
	mCamera->setCullMask(VisibilityCategory::shadowCaster); // Only render shadow casters

	addChild(mCamera);

	setRadiusWorldSpace(mRadiusWorldSpace);

	osg::StateSet* ss = mCamera->getOrCreateStateSet();

	// Caster uniforms
	mCameraPositionUniform = new osg::Uniform("cameraPosition", osg::Vec3f(0, 0, 0));
	ss->addUniform(mCameraPositionUniform);

	mViewMatrixUniform = new osg::Uniform("viewMatrix", osg::Matrixf());
	ss->addUniform(mViewMatrixUniform);

	mViewProjectionMatrixUniform = new osg::Uniform("viewProjectionMatrix", osg::Matrixf());
	ss->addUniform(mViewProjectionMatrixUniform);

	// Receiver uniforms
	mShadowProjectionMatrixUniform = new osg::Uniform(std::string("shadowProjectionMatrix" + std::to_string(config.shadowMapId)).c_str(), osg::Matrixf());
}

void ShadowMapGenerator::setRadiusWorldSpace(double radius)
{
	mRadiusWorldSpace = radius;
	float nearClip = 1;
	float farClip = mDepthRangeWorldSpace;
	mCamera->setProjectionMatrixAsOrtho(-mRadiusWorldSpace, mRadiusWorldSpace, -mRadiusWorldSpace, mRadiusWorldSpace, nearClip, farClip);
}

void ShadowMapGenerator::setDepthRangeWorldSpace(float range)
{
	mDepthRangeWorldSpace = range;
	setRadiusWorldSpace(mRadiusWorldSpace);
}

void ShadowMapGenerator::update(const osg::Vec3& shadowCameraPosition, const osg::Vec3& lightDirection, const osg::Vec3& wrappedNoiseOrigin)
{
	osg::Vec3 tangent, bitangent;
	getOrthonormalBasis(lightDirection, tangent, bitangent);
	osg::Matrix m = makeMatrixFromTbn(tangent, bitangent, lightDirection);
	m.setTrans(shadowCameraPosition);

	osg::Matrix viewMatrix = osg::Matrix::inverse(m);

	// Quantize view matrix to the nearest texel to avoid jittering artifacts
	{
		osg::Vec4 originInShadowSpace = osg::Vec4(wrappedNoiseOrigin, 1.0) * viewMatrix;
		osg::Vec3 translation;

		osg::Vec2f shadowWorldTexelSize(2.0f * mRadiusWorldSpace / (float)mTexture->getTextureWidth(), 2.0f * mRadiusWorldSpace / (float)mTexture->getTextureHeight());

		translation.x() = std::floor(originInShadowSpace.x() / shadowWorldTexelSize.x()) * shadowWorldTexelSize.x() - originInShadowSpace.x();
		translation.y() = std::floor(originInShadowSpace.y() / shadowWorldTexelSize.y()) * shadowWorldTexelSize.y() - originInShadowSpace.y();
		translation.z() = 0.0;

		viewMatrix.postMultTranslate(translation);
	}

	mCamera->setViewMatrix(viewMatrix);

	mCameraPositionUniform->set(osg::Vec3f(shadowCameraPosition));

	mViewMatrixUniform->set(mCamera->getViewMatrix());

	osg::Matrixf viewProj = mCamera->getViewMatrix() * mCamera->getProjectionMatrix();
	mViewProjectionMatrixUniform->set(viewProj);

	osg::Matrixf shadowMatrix(
		0.5, 0.0, 0.0, 0.0,
		0.0, 0.5, 0.0, 0.0,
		0.0, 0.0, 0.5, 0.0,
		0.5, 0.5, 0.5, 1.0
	);

	osg::Matrix shadowProjectionMatrix = viewProj * shadowMatrix;
	mShadowProjectionMatrixUniform->set(shadowProjectionMatrix);
}

void ShadowMapGenerator::configureShadowReceiverStateSet(osg::StateSet& ss)
{
	ss.addUniform(mShadowProjectionMatrixUniform);
}

void ShadowMapGenerator::setScene(const osg::ref_ptr<osg::Node>& scene)
{
	mCamera->removeChild(0, mCamera->getNumChildren());
	mCamera->addChild(scene);
}

osg::Matrix ShadowMapGenerator::getShadowProjectionMatrix() const
{
	osg::Matrix mat;
	mShadowProjectionMatrixUniform->get(mat);
	return mat;
}

} // namespace vis
} // namespace skybolt
