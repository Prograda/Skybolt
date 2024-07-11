/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#pragma once

#include "SkyboltVis/DefaultRootNode.h"

namespace skybolt {
namespace vis {

struct PlanetSkyConfig
{
	osg::ref_ptr<osg::Program> program;
	float radius;
};

class PlanetSky : public DefaultRootNode
{
public:
	PlanetSky(const PlanetSkyConfig& config);
	~PlanetSky();
	
	void setOrientation(const osg::Quat& orientation) {}; //!< Has no effect

private:
	osg::Geode* mGeode;
	float radius;
};

} // namespace vis
} // namespace skybolt
