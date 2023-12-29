/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace skybolt {
namespace sim {

typedef glm::dvec3 Vector3;
typedef glm::dquat Quaternion;
typedef glm::dmat3 Matrix3;
typedef glm::dmat4 Matrix4;

//! Returns tangent and bitangent perpendiular to normal and each other
void getOrthonormalBasis(const Vector3 &normal, Vector3 &tangent, Vector3 &bitangent);

//! Generates an orientation from a direction, with arbitrary roll angle
Matrix3 getOrientationFromDirection(const Vector3& direction);

double posInfinity();
double negInfinity();

} // namespace sim
} // namespace skybolt