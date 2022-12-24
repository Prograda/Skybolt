/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltVis/DefaultRootNode.h"
#include "SkyboltVis/Renderable/ScreenQuad.h"
#include <osg/PrimitiveSet>
#include <osg/Texture2D>

#include <functional>

namespace skybolt {
namespace vis {

struct VolumeCloudsCompositeConfig
{
	osg::ref_ptr<osg::Program> compositorProgram;
	osg::ref_ptr<osg::Texture> colorTexture;
	osg::ref_ptr<osg::Texture> depthTexture;
};

class VolumeCloudsComposite : public ScreenQuad
{
public:
	VolumeCloudsComposite(const VolumeCloudsCompositeConfig& config);

	void setColorTexture(osg::ref_ptr<osg::Texture> texture);
	void setDepthTexture(osg::ref_ptr<osg::Texture> texture);
};

} // namespace vis
} // namespace skybolt
