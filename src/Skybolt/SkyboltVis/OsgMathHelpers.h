/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <stddef.h> // for size_t

namespace skybolt {
namespace math {

template <typename T>
constexpr size_t componentCount(const T& v)
{
	return T::num_components;
}

} // namespace math
} // namespace skybolt
