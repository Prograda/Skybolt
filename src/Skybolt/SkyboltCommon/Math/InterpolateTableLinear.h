/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltCommon/Range.h"
#include <algorithm>
#include <optional>
#include <vector>

namespace skybolt {
namespace math {

struct InterpolationPoint
{
	IntRangeClosedOpen bounds;
	double weight; //!< In range [0 to 1]
};

//! Returns null if the input vector is empty, otherwise returns a valid result.
std::optional<InterpolationPoint> findInterpolationPoint(const std::vector<double> &xData, double x, bool extrapolate);

//! Returns null if the input vectors is empty, otherwise returns a valid result.
//! xData and yData must be the same length.
std::optional<double> interpolateTableLinear(const std::vector<double> &xData, const std::vector<double> &yData, double x, bool extrapolate);

} // namespace math
} // namespace skybolt
