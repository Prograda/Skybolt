/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "HeightMapElevationRerange.h"

#include <osg/Image>
#include <osg/ValueObject>

namespace skybolt {
namespace vis {

int getColorValueForElevation(const HeightMapElevationRerange& rerange, float elevation)
{
	return (elevation - rerange.y()) / rerange.x();
}

float getElevationForColorValue(const HeightMapElevationRerange& rerange, int value)
{
	return value * rerange.x() + rerange.y();
}

std::optional<HeightMapElevationRerange> getHeightMapElevationRerange(const osg::Image& image)
{
	HeightMapElevationRerange rerange;
	if (image.getUserValue<HeightMapElevationRerange>("HeightMapElevationRerange", rerange))
	{
		return rerange;
	}
	return std::nullopt;
}

HeightMapElevationRerange getRequiredHeightMapElevationRerange(const osg::Image& image)
{
	std::optional<HeightMapElevationRerange> r = getHeightMapElevationRerange(image);
	if (!r)
	{
		throw std::runtime_error("HeightMap does not have associated elevation rerange data");
	}
	return *r;
}

void setHeightMapElevationRerange(osg::Image& image, const HeightMapElevationRerange& rerange)
{
	image.setUserValue("HeightMapElevationRerange", rerange);
}

} // namespace vis
} // namespace skybolt