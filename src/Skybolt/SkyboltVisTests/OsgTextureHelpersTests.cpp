/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include <catch2/catch.hpp>
#include "SkyboltVis/OsgMathHelpers.h"
#include <SkyboltVis/OsgTextureHelpers.h>
#include <SkyboltCommon/Math/MathUtility.h>

using namespace skybolt;
using namespace skybolt::vis;

TEST_CASE("calcHalfTexelEdgeRemovalScaleOffset produces scale and offset which transform texture to have half-texel edge removed")
{
	ScaleOffset scaleOffset = calcHalfTexelEdgeRemovalScaleOffset(osg::Vec2i(4, 8));

	osg::Vec2f q;
	q = math::componentWiseMultiply(scaleOffset.scale, osg::Vec2f(0, 0)) + scaleOffset.offset;	
	CHECK(q.x() == 0.5f / 4.f);
	CHECK(q.y() == 0.5f / 8.f);

	q = math::componentWiseMultiply(scaleOffset.scale, osg::Vec2f(1, 1)) + scaleOffset.offset;	
	CHECK(q.x() == 3.5f / 4.f);
	CHECK(q.y() == 7.5f / 8.f);
}