/* Copyright 2012-2020 Matthew Reid
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
	Box3T(const T& minimum, const T& maximum) :
	minimum(minimum), maximum(maximum)
	{
	}

	T minimum;
	T maximum;
};

using Box3 = Box3T<glm::vec3>;

} // namespace skybolt
