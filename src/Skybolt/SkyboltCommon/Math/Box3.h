/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <glm/glm.hpp>

namespace skybolt
{ 

template <typename T>
struct Box3T
{
	Box3T() :
		minimum(std::numeric_limits<typename T::value_type>::max(), std::numeric_limits<typename T::value_type>::max(), std::numeric_limits<typename T::value_type>::max()),
		maximum(std::numeric_limits<typename T::value_type>::lowest(), std::numeric_limits<typename T::value_type>::lowest(), std::numeric_limits<typename T::value_type>::lowest())
	{}

	Box3T(const T& minimum, const T& maximum) :
	minimum(minimum), maximum(maximum)
	{
	}

	inline T size() const { return maximum - minimum; }
	inline T center() const { return (maximum + minimum) / (typename T::value_type)(2); }

	inline void merge(const T& v)
	{
		if (v[0] < minimum[0]) minimum[0] = v[0];
		if (v[1] < minimum[1]) minimum[1] = v[1];
		if (v[2] < minimum[2]) minimum[2] = v[2];

		if (v[0] > maximum[0]) maximum[0] = v[0];
		if (v[1] > maximum[1]) maximum[1] = v[1];
		if (v[2] > maximum[2]) maximum[2] = v[2];
	}

	T minimum;
	T maximum;
};

using Box3 = Box3T<glm::vec3>;
using Box3d = Box3T<glm::dvec3>;

} // namespace skybolt
