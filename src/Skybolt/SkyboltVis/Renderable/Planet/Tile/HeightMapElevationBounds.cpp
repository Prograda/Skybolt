/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "HeightMapElevationBounds.h"

#include <osg/Image>
#include <osg/ValueObject>

namespace skybolt {
namespace vis {

std::optional<HeightMapElevationBounds> getHeightMapElevationBounds(const osg::Image& image)
{
	HeightMapElevationBounds bounds;
	if (image.getUserValue<HeightMapElevationBounds>("HeightMapElevationBounds", bounds))
	{
		return bounds;
	}
	return std::nullopt;
}

HeightMapElevationBounds getRequiredHeightMapElevationBounds(const osg::Image& image)
{
	std::optional<HeightMapElevationBounds> r = getHeightMapElevationBounds(image);
	if (!r)
	{
		throw std::runtime_error("HeightMap does not have associated elevation bounds data");
	}
	return *r;
}

void setHeightMapElevationBounds(osg::Image& image, const HeightMapElevationBounds& bounds)
{
	image.setUserValue("HeightMapElevationBounds", bounds);
}

} // namespace vis
} // namespace skybolt