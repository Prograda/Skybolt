/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "ElevationProvider.h"
#include "ElevationRerange.h"
#include "SkyboltVis/SkyboltVisFwd.h"
#include <SkyboltCommon/Math/Box2.h>

namespace skybolt {
namespace vis {

class HeightMapElevationProvider : public ElevationProvider
{
public:
	HeightMapElevationProvider(const ImagePtr& image, const ElevationRerange& elevationRerange, const Box2d& bounds);

	//! @param x is latitude in radians
	//! @param y is longitude in radians
	float get(double x, double y) const;

private:
	ImagePtr mImage;
	ElevationRerange mElevationRerange;
	const glm::dvec2 mHorizontalOffset;
	glm::dvec2 mHorizontalScale;
};

} // namespace vis
} // namespace skybolt
