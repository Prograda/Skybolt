/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltVis/DefaultRootNode.h"
#include <osg/PrimitiveSet>
#include <osg/Texture2D>

namespace skybolt {
namespace vis {

struct VolumeCloudsConfig
{
	osg::ref_ptr<osg::Program> program;
	float innerCloudLayerRadius;
	float outerCloudLayerRadius;
	osg::ref_ptr<osg::Texture2D> cloudsTexture;
};

class VolumeClouds : public DefaultRootNode
{
public:
	VolumeClouds(const VolumeCloudsConfig& config);
	~VolumeClouds();

	struct Uniforms
	{
		osg::Uniform* modelMatrixUniform;
		osg::Uniform* topLeftDir;
		osg::Uniform* topRightDir;
		osg::Uniform* bottomLeftDir;
		osg::Uniform* bottomRightDir;
	};

	void updatePreRender(const CameraRenderContext& context) override;

private:
	osg::ref_ptr<osg::Geode> mGeode;
	Uniforms mUniforms;
};

} // namespace vis
} // namespace skybolt
