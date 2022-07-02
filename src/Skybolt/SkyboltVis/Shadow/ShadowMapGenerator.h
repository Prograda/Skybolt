/* Copyright 2012-2021 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltVis/RenderOperation/RenderOperation.h"
#include <osg/Camera>
#include <osg/Group>
#include <osg/Program>
#include <osg/Texture2D>

namespace skybolt {
namespace vis {

struct ShadowMapGeneratorConfig
{
	int textureSize = 1024;
	int shadowMapId = 0;
};

class ShadowMapGenerator : public RenderOperation
{
public:
	ShadowMapGenerator(const ShadowMapGeneratorConfig& config);

	void update(const osg::Vec3& shadowCameraPosition, const osg::Vec3& lightDirection, const osg::Vec3& wrappedNoiseOrigin);
	float getRadiusWorldSpace() const { return mRadiusWorldSpace; }
	void setRadiusWorldSpace(double radius);

	float getDepthRangeWorldSpace() { return mDepthRangeWorldSpace; }
	void setDepthRangeWorldSpace(float range);

	void configureShadowReceiverStateSet(osg::StateSet& ss);

	osg::ref_ptr<osg::Texture2D> getTexture() const { return mTexture; }

	void setScene(const osg::ref_ptr<osg::Node>& scene);

	osg::Matrix getShadowProjectionMatrix() const;

public: // RenderOperation interface
	std::vector<osg::ref_ptr<osg::Texture>> getOutputTextures() const override
	{
		return { mTexture };
	}

private:
	osg::ref_ptr<osg::Texture2D> mTexture;
	osg::ref_ptr<osg::Camera> mCamera;
	osg::ref_ptr<osg::Uniform> mCameraPositionUniform;
	osg::ref_ptr<osg::Uniform> mViewMatrixUniform;
	osg::ref_ptr<osg::Uniform> mViewProjectionMatrixUniform;
	osg::ref_ptr<osg::Uniform> mShadowProjectionMatrixUniform;
	float mRadiusWorldSpace = 2000;
	float mDepthRangeWorldSpace = 10000;
};

} // namespace vis
} // namespace skybolt
