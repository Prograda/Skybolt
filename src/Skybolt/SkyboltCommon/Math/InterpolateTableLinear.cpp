/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "InterpolateTableLinear.h"
#include "MathUtility.h"

namespace skybolt {
namespace math {

std::optional<InterpolationPoint> findInterpolationPoint(const std::vector<double> &xData, double x, bool extrapolate)
{
	int size = (int)xData.size();
	if (size == 0)
	{
		return std::nullopt;
	}
	else if (size == 1)
	{
		InterpolationPoint point;
		point.bounds.first = 0;
		point.bounds.last = 0;
		point.weight = 0;
		return point;
	}

	// Find left bound
	int i = 0;                                                                  
	if (x >= xData[size - 2]) // Make sure we're not past the right bound
	{
		i = size - 2;
	}
	else
	{
		while (x > xData[i + 1])
		{
			i++;
		}
	}
	double xL = xData[i];
	double xR = xData[i + 1];

	InterpolationPoint point;
	point.bounds.first = i;
	point.bounds.last = i + 1;
	point.weight = (x - xL) / (xR - xL);
	
	if (!extrapolate)
	{
		point.weight = math::clamp(point.weight, 0.0, 1.0);
	}

	return point;
}

std::optional<double> interpolateTableLinear(const std::vector<double> &xData, const std::vector<double> &yData, double x, bool extrapolate)
{
	std::optional<InterpolationPoint> point = findInterpolationPoint(xData, x, extrapolate);
	if (!point)
	{
		return std::nullopt;
	}
	return math::lerp(xData[point->bounds.first], xData[point->bounds.last], point->weight);
}

} // namespace math
} // namespace skybolt
