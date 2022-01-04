/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "MathUtility.h"
#include <glm/glm.hpp>
#include <limits>

namespace skybolt {

template <typename T>
struct Box2T
{
	Box2T() :
		minimum(std::numeric_limits<typename T::value_type>::max(), std::numeric_limits<typename T::value_type>::max()),
		maximum(std::numeric_limits<typename T::value_type>::lowest(), std::numeric_limits<typename T::value_type>::lowest())
	{}

	Box2T(const T& minimum, const T& maximum) : minimum(minimum), maximum(maximum) {}

	inline T size() const { return maximum - minimum; }
	inline T center() const { return (maximum + minimum) / (typename T::value_type)(2); }

	inline bool intersects(const T& v) const
	{
		return v[0] >= minimum[0] && v[0] <= maximum[0] && v[1] >= minimum[1] && v[1] <= maximum[1];
	}

	inline bool intersects(const Box2T<T>& b) const
	{
		return (maximum[0] >= b.minimum[0] && b.maximum[0] >= minimum[0] &&
			    maximum[1] >= b.minimum[1] && b.maximum[1] >= minimum[1]);
	}

	inline void merge(const T& v)
	{
		if (v[0] < minimum[0]) minimum[0] = v[0];
		if (v[1] < minimum[1]) minimum[1] = v[1];

		if (v[0] > maximum[0]) maximum[0] = v[0];
		if (v[1] > maximum[1]) maximum[1] = v[1];
	}

	inline void merge(const Box2T<T>& b)
	{
		if (b.minimum[0] < minimum[0]) minimum[0] = b.minimum[0];
		if (b.minimum[1] < minimum[1]) minimum[1] = b.minimum[1];
		
		if (b.maximum[0] > maximum[0]) maximum[0] = b.maximum[0];
		if (b.maximum[1] > maximum[1]) maximum[1] = b.maximum[1];
	}

	//! @param p is a vector with compoents in range [0, 1], where 0 is at minimum bound and 1 is at maximum bound.
	T getPointFromNormalizedCoord(const T& p) const
	{
		return math::componentWiseLerp(minimum, maximum, p);
	}

	//! Returns vector with compoents in range [0, 1], where 0 is at minimum bound and 1 is at maximum bound.
	T getNormalizedCoordinate(const T& p) const
	{
		return math::componentWiseDivide((p - minimum), size());
	}

	static Box2T<T> unitBox()
	{
		return Box2T<T>(T(0, 0), T(1, 1));
	}

	T minimum;
	T maximum;
};

using Box2 = Box2T<glm::vec2>;
using Box2i = Box2T<glm::ivec2>;

} // namespace skybolt
