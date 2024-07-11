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

class Polyline : public DefaultRootNode
{
public:
	struct Params
	{
		osg::ref_ptr<osg::Program> program;
	};

	Polyline(const Params& params);
	~Polyline();

	void setPoints(const osg::ref_ptr<osg::Vec3Array>& points = nullptr);

	void setColor(const osg::Vec4f& color);

	enum class LineMode
	{
		Strip,
		Lines
	};

	void setLineMode(LineMode lineMode) { mLineMode = lineMode; }

private:
	osg::Geometry* mGeometry;
	osg::Geode* mGeode;
	osg::Vec4f mColor = osg::Vec4f(0,1,0,1);
	LineMode mLineMode = LineMode::Strip;
};

} // namespace vis
} // namespace skybolt
