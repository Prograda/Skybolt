/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#pragma once

#include "SkyboltVis/DefaultRootNode.h"

namespace skybolt {
namespace vis {

struct StarfieldConfig
{
	osg::ref_ptr<osg::Program> program;
};

class Starfield : public DefaultRootNode
{
public:
	Starfield(const StarfieldConfig& config);
	~Starfield();

private:
	void updatePreRender(const CameraRenderContext& context);

private:
	osg::Geode* mGeode;
	osg::Uniform* mBrightnessUniform;
};

} // namespace vis
} // namespace skybolt
