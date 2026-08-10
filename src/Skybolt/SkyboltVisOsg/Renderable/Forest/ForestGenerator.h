/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "BillboardForest.h"
#include "SkyboltVis/Elevation/ElevationProvider.h"
#include <SkyboltCommon/Random.h>
#include <SkyboltCommon/Math/Box2.h>
#include <osg/Vec2f>
#include <osg/Vec3f>
#include <osg/Array>
#include <map>

namespace skybolt {
namespace vis {

class ForestGenerator
{
public:
	struct Attribute
	{
		float density; //!< trees per square meter
	};

	typedef std::map<int, Attribute> Attributes;

	struct AttributeImage
	{
		char* data; //!< origin is bottom left
		int width;
		int height;
		Attributes attributes;
	};

	ForestGenerator();

	//! @param imageBounds are inclusive
	std::vector<BillboardForest::Tree> generate(const ElevationProvider& elevation, const AttributeImage& image, const Box2d& worldBounds, const Box2d& imageBounds);

private:
	skybolt::Random random;
};

} // namespace vis
} // namespace skybolt
