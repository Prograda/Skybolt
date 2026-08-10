/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <SkyboltCommon/Math/Box2.h>

namespace skybolt {
namespace vis {

inline Box2d getSubImageBounds(const Box2d& imageWorldBounds, const Box2d& subRegionWorldBounds, int imageWidth, int imageHeight)
{
	glm::dvec2 size = imageWorldBounds.maximum - imageWorldBounds.minimum;
	double widthF = double(imageWidth);
	double heightF = double(imageHeight);

	Box2d imageBounds;
	imageBounds.minimum.x = glm::clamp(widthF * (subRegionWorldBounds.minimum.y - imageWorldBounds.minimum.y) / size.y, 0.0, widthF);
	imageBounds.minimum.y = glm::clamp(heightF * (subRegionWorldBounds.minimum.x - imageWorldBounds.minimum.x) / size.x, 0.0, heightF);
	imageBounds.maximum.x = glm::clamp(widthF * (subRegionWorldBounds.maximum.y - imageWorldBounds.minimum.y) / size.y, 0.0, widthF);
	imageBounds.maximum.y = glm::clamp(heightF * (subRegionWorldBounds.maximum.x - imageWorldBounds.minimum.x) / size.x, 0.0, heightF);

	return imageBounds;
}

} // namespace vis
} // namespace skybolt
