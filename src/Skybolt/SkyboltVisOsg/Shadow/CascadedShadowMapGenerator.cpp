/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "CascadedShadowMapGenerator.h"
#include "Camera.h"
#include "ShadowMapGenerator.h"
#include <assert.h>
#include <algorithm>

namespace skybolt {
namespace vis {

static std::vector<float> calculateSplitDistances(size_t cascadeCount, float nearDistance, float farDistance, float lambda = 0.8)
{
	assert(cascadeCount >= 1);

	// Calculate split distances using technique from GPU Gems 3.
	// https://developer.nvidia.com/gpugems/gpugems3/part-ii-light-and-shadows/chapter-10-parallel-split-shadow-maps-programmable-gpus
	std::vector<float> splitDistances(cascadeCount - 1);

	for (size_t i = 0; i < splitDistances.size(); i++)
	{
		float fraction = float(i + 1) / cascadeCount;
		float logDistance = nearDistance * std::pow(farDistance / nearDistance, fraction);
		float linDistance = nearDistance + (farDistance - nearDistance) * fraction;
		float splitDistance = linDistance + lambda * (logDistance - linDistance);

		splitDistances[i] = splitDistance;
	}
	return splitDistances;
}

static std::vector<float> calculateCascadeBoundingDistances(size_t cascadeCount, double maxRange)
{
	// Automatic split calculation. TODO: improve results
	static std::vector<float> cascadeBoundingDistances = calculateSplitDistances(cascadeCount, 0.1f, maxRange);
	cascadeBoundingDistances.insert(cascadeBoundingDistances.begin(), 0);
	cascadeBoundingDistances.push_back(maxRange);
	return cascadeBoundingDistances;
}

CascadedShadowMapGenerator::CascadedShadowMapGenerator(const CascadedShadowMapGeneratorConfig& config) :
	mCascadeBoundingDistances(config.cascadeBoundingDistances)
{
	int cascadeCount = config.cascadeBoundingDistances.size() - 1;
	assert(cascadeCount >= 1);

	mCascadeShadowMatrixModifierUniform = new osg::Uniform(osg::Uniform::FLOAT_VEC4, "cascadeShadowMatrixModifier", cascadeCount);

	for (int i = 0; i < cascadeCount; ++i)
	{
		ShadowMapGeneratorConfig c;
		c.textureSize = config.textureSize;
		c.shadowMapId = i;
		osg::ref_ptr<ShadowMapGenerator> generator = new ShadowMapGenerator(c);
		mTextures.push_back(generator->getTexture());
		mShadowMapGenerators.push_back(generator);
	}

	mCascadeTexelDepthSizesUniform = osg::ref_ptr<osg::Uniform>(new osg::Uniform("cascadeTexelDepthSizes", osg::Vec4f(0,0,0,0)));
	mMaxShadowViewDistance = osg::ref_ptr<osg::Uniform>(new osg::Uniform("maxShadowViewDistance", 0.0f));
}

void CascadedShadowMapGenerator::update(const vis::Camera& viewCamera, const osg::Vec3& lightDirection, const osg::Vec3& wrappedNoiseOrigin)
{
	mMaxShadowViewDistance->set(mCascadeBoundingDistances.back());

	for (size_t i = 0; i < mShadowMapGenerators.size(); ++i)
	{
		const auto& generator = mShadowMapGenerators[i];

		Frustum frustum;
		frustum.fieldOfViewY = viewCamera.getFovY();
		frustum.aspectRatio = viewCamera.getAspectRatio();
		frustum.nearPlaneDistance = mCascadeBoundingDistances[i];
		frustum.farPlaneDistance = mCascadeBoundingDistances[i + 1];
		auto result = calculateMinimalEnclosingSphere(frustum);

		double radiusPaddingMultiplier = 1.05; // make cascade slightly bigger to ensure enough overlap for blending between cascades. TODO: calculate actual overlap required.
		generator->setRadiusWorldSpace(result.radius * radiusPaddingMultiplier);
		generator->setDepthRangeWorldSpace(mCascadeBoundingDistances.back() * 2);

		osg::Vec3 viewCameraDirection = viewCamera.getOrientation() * osg::Vec3f(1, 0, 0);
		osg::Vec3 shadowCameraPosition = viewCamera.getPosition() + viewCameraDirection * result.centerDistance + lightDirection * generator->getDepthRangeWorldSpace() * 0.5f;
		generator->update(shadowCameraPosition, lightDirection, wrappedNoiseOrigin);
	}


	for (size_t i = 0; i < mShadowMapGenerators.size(); ++i)
	{
		mCascadeShadowMatrixModifierUniform->setElement(i, calculateCascadeShadowMatrixModifier(i));
	}

	assert(mShadowMapGenerators.size() <= 4);
	osg::Vec4f cascadeTexelSizes;
	for (size_t i = 0; i < std::min(4, int(mShadowMapGenerators.size())); ++i)
	{
		auto generator = mShadowMapGenerators[i];
		cascadeTexelSizes[i] = 2.0f * generator->getRadiusWorldSpace() / (generator->getTexture()->getTextureWidth() * generator->getDepthRangeWorldSpace());
	}
	mCascadeTexelDepthSizesUniform->set(cascadeTexelSizes);
}

void CascadedShadowMapGenerator::setScene(const osg::ref_ptr<osg::Node>& scene)
{
	for (const auto& generator : mShadowMapGenerators)
	{
		generator->setScene(scene);
	}
}

osg::Vec4 CascadedShadowMapGenerator::calculateCascadeToCascadeTransform(const osg::Matrix m0, const osg::Matrix m1)
{
	float scale = m1.getScale().x() / m0.getScale().x();

	float offsetX = m1(3, 0) - m0(3, 0) * scale;
	float offsetY = m1(3, 1) - m0(3, 1) * scale;
	float offsetZ = m1(3, 2) - m0(3, 2);

	return osg::Vec4f(
		scale,
		offsetX,
		offsetY,
		offsetZ
	);
}

CascadedShadowMapGenerator::FrustumMinimalEnclosingSphere CascadedShadowMapGenerator::calculateMinimalEnclosingSphere(const Frustum& frustum)
{
	// Technique from https://lxjk.github.io/2017/04/15/Calculate-Minimal-Bounding-Sphere-of-Frustum.html
	float k = std::sqrt(1 + frustum.aspectRatio * frustum.aspectRatio) * std::tan(frustum.fieldOfViewY / 2.f);
	float k2 = k * k;
	float k4 = k2 * k2;

	if (k*k >= (frustum.farPlaneDistance - frustum.nearPlaneDistance) / (frustum.farPlaneDistance + frustum.nearPlaneDistance))
	{
		return { frustum.farPlaneDistance, frustum.farPlaneDistance * k };
	}
	else
	{
		float fPlusN = frustum.farPlaneDistance + frustum.nearPlaneDistance;
		float centerDistance = 0.5f * fPlusN * (1 + k2);
		float radius = 0.5f * std::sqrt(
			osg::square(frustum.farPlaneDistance - frustum.nearPlaneDistance)
			+ 2 * ( osg::square(frustum.farPlaneDistance) + osg::square(frustum.nearPlaneDistance)) * k2
			+ osg::square(fPlusN) * k4
		);
		return { centerDistance, radius };
	}
}

osg::Vec4f CascadedShadowMapGenerator::calculateCascadeShadowMatrixModifier(int i) const
{
	osg::Matrix mat0 = mShadowMapGenerators[0]->getShadowProjectionMatrix();
	osg::Matrix mat = mShadowMapGenerators[i]->getShadowProjectionMatrix();

	return calculateCascadeToCascadeTransform(mat0, mat);
}

void CascadedShadowMapGenerator::configureShadowReceiverStateSet(osg::StateSet& ss)
{
	for (const auto& generator : mShadowMapGenerators)
	{
		generator->configureShadowReceiverStateSet(ss);
	}

	ss.addUniform(mCascadeShadowMatrixModifierUniform);
	ss.addUniform(mCascadeTexelDepthSizesUniform);
	ss.addUniform(mMaxShadowViewDistance);
}

} // namespace vis
} // namespace skybolt
