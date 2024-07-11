/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#pragma once

#include "SkyboltVis/DefaultRootNode.h"
#include <osg/Program>

namespace skybolt {
namespace vis {

struct Vec3Segment
{
	osg::Vec3 start;
	osg::Vec3 end;
};

class Arrows : public DefaultRootNode
{
public:
	struct Params
	{
		osg::ref_ptr<osg::Program> program;
	};

	Arrows(const Params& params);
	~Arrows();

	void setSegments(const std::vector<Vec3Segment>& segments);

private:
	osg::Geometry* mGeometry;
	osg::Geode* mGeode;
};

} // namespace vis
} // namespace skybolt
