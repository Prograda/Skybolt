/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#pragma once

#include <glm/glm.hpp>

namespace skybolt {

// Interpolate over [0, 1] from initial value to signal value
template <typename TimeT>
TimeT calcFirstOrderLagInterpolationFactor(const TimeT& dt, const TimeT& timeConstant)
{
	if (timeConstant <= 0)
	{
		return 1.0;
	}

	return TimeT(1.0) - std::exp(-dt / timeConstant);
}

//! @param timeConstant is the time for the system's step response to reach 63.2% of the signal value
template <typename ValueT, typename TimeT>
ValueT firstOrderLag(const ValueT& initial, const ValueT& signal, const TimeT& dt, const TimeT& timeConstant)
{
	TimeT f = calcFirstOrderLagInterpolationFactor(dt, timeConstant);
	return glm::mix(initial, signal, f);
}

} // namespace skybolt
