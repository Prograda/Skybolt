/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "Interpolator.h"
#include <glm/glm.hpp>
#include <functional>

namespace skybolt {

template <typename T>
class LinearInterpolator : public Interpolator<T>
{
public:
	typedef std::function<T(int index)> ArrayValueGetter;

	LinearInterpolator(const ArrayValueGetter& valueGetter) :
		mValueGetter(valueGetter)
	{
	}

	//! @param u is the parameteric interpolation coordinate in range [0, 1]
	T interpolate(int firstIndex, int secondIndex, double u) const override
	{
		return glm::mix(mValueGetter(firstIndex), mValueGetter(secondIndex), u);
	}

private:
	ArrayValueGetter mValueGetter;
};

typedef LinearInterpolator<double> LinearInterpolatorD;

} // namespace skybolt