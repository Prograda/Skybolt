/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltVis/DefaultRootNode.h"
#include "SkyboltVis/OsgBox2.h"

namespace skybolt {
namespace vis {

class ScreenQuad : public DefaultRootNode
{
public:
	ScreenQuad(osg::StateSet* stateSet, const BoundingBox2f& bounds = BoundingBox2f(osg::Vec2f(0, 0), osg::Vec2f(1, 1)));

	static osg::ref_ptr<osg::Geode> createGeode(osg::StateSet* stateSet, const BoundingBox2f& bounds = BoundingBox2f(osg::Vec2f(0, 0), osg::Vec2f(1, 1)));
	static osg::ref_ptr<osg::Group> createNode(osg::StateSet* stateSet, const BoundingBox2f& bounds = BoundingBox2f(osg::Vec2f(0, 0), osg::Vec2f(1, 1)));
};

} // namespace vis
} // namespace skybolt
