/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltVis/SkyboltVisFwd.h"
#include <osg/Group>
#include <osg/Program>
#include <osg/Texture2D>

namespace skybolt {
namespace vis {

struct CascadedShadowMapGeneratorConfig
{
	int textureSize;
	std::vector<float> cascadeBoundingDistances; //!< size is number of cascades + 1
};

class CascadedShadowMapGenerator
{
public:
	CascadedShadowMapGenerator(const CascadedShadowMapGeneratorConfig& config);

	//! @param lightDirection is the direction to the light
	void update(const vis::Camera& viewCamera, const osg::Vec3& lightDirection, const osg::Vec3& wrappedNoiseOrigin);

	void configureShadowReceiverStateSet(osg::StateSet& ss);

	std::vector<osg::ref_ptr<osg::Texture2D>> getTextures() const { return mTextures; }

	std::vector<osg::ref_ptr<ShadowMapGenerator>> getGenerators() const { return mShadowMapGenerators; }

	void setScene(const osg::ref_ptr<osg::Node>& scene);

	static osg::Vec4 calculateCascadeToCascadeTransform(const osg::Matrix m0, const osg::Matrix m1);

	struct Frustum
	{
		float nearPlaneDistance;
		float farPlaneDistance;
		float fieldOfViewY; //!< field of view about Y axis in radians
		float aspectRatio; //!< width / height
	};

	struct FrustumMinimalEnclosingSphere
	{
		double centerDistance; //! distance of sphere center from frustum origin along frustum's center ray
		double radius; //!< radius of sphere
	};

	static FrustumMinimalEnclosingSphere calculateMinimalEnclosingSphere(const Frustum& frustum);

private:
	osg::Vec4f calculateCascadeShadowMatrixModifier(int i) const;

private:
	std::vector<osg::ref_ptr<osg::Texture2D>> mTextures;
	std::vector<osg::ref_ptr<ShadowMapGenerator>> mShadowMapGenerators;
	std::vector<float> mCascadeBoundingDistances;

	//! Vec4f array. Stores a Vec4f for each cascade with values [scaleXY, offsetX, offsetY, offsetZ] relative to the first cascade.
	//! This is used by the shader to avoid having multiplying the shaded point by the matrix of every cascade.
	//! Instead, the higher cascades transformation is derived from the first cascade's matrix multiplied result.
	osg::ref_ptr<osg::Uniform> mCascadeShadowMatrixModifierUniform;

	//! World sizes of texels for each cascade, divided by shadow camera view range.
	//! Used to calculage depth offset bias.
	//! Each component of the vector stores valid for a different cascaide in ascending order, i.e [0, 1, 2, 3]
	osg::ref_ptr<osg::Uniform> mCascadeTexelDepthSizesUniform;

	osg::ref_ptr<osg::Uniform> mMaxShadowViewDistance; //!< Maximum distance at which to display shadows
};

} // namespace vis
} // namespace skybolt
