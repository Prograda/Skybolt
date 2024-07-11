/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <osg/Vec3>

#include <stddef.h> // for size_t

namespace skybolt {
namespace math {

template <typename T>
constexpr size_t componentCount(const T& v)
{
	return T::num_components;
}

void getOrthonormalBasis(const osg::Vec3f &normal, osg::Vec3f &tangent, osg::Vec3f &bitangent);

} // namespace math
} // namespace skybolt
