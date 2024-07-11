/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

namespace skybolt {

template <typename T>
class Interpolator
{
public:
	virtual ~Interpolator() {}

	//! @param u is the parameteric interpolation coordinate in range [0, 1]
	virtual T interpolate(int firstIndex, int secondIndex, double u) const = 0;
};

typedef Interpolator<double> InterpolatorD;

} // namespace skybolt