/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltVis/DefaultRootNode.h"

#include <osg/ClipPlane>
#include <osg/Texture2D>

namespace skybolt {
namespace vis {

struct OceanConfig
{
	osg::ref_ptr<osg::Program> oceanProgram;
	osg::ref_ptr<osg::StateSet> waterStateSet;
};

class Ocean : public DefaultRootNode
{
public:
	Ocean(const OceanConfig& config);
	~Ocean();

	struct Uniforms
	{
		osg::Uniform* topLeftDir;
		osg::Uniform* topRightDir;
		osg::Uniform* bottomLeftDir;
		osg::Uniform* bottomRightDir;
		osg::Uniform* waterHeight;
	};

	void setPosition(const osg::Vec3f& position);
	void setOrientation(const osg::Quat& orientation) {}; //!< Has no effect

private:
	void updatePreRender(const CameraRenderContext& context);

private:
	Uniforms mUniforms;
	osg::Node* mGrid;
};

} // namespace vis
} // namespace skybolt
