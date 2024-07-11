/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <cmath>

namespace skybolt {

template <typename T>
bool almostEqual(T expected, T value, double maxError)
{
	return (std::abs(expected - value) < maxError);
}

template <typename T>
bool almostEqualFracEpsilon(T expected, T value, double maxError)
{
	return almostEqual(expected, value, expected * maxError);
}

} // namespace skybolt
