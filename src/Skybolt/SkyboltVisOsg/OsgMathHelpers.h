/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <osg/Quat>
#include <osg/Vec2>
#include <osg/Vec2d>
#include <osg/Vec3>
#include <osg/Vec3d>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <stddef.h> // for size_t

namespace skybolt {
namespace math {

template <typename T>
constexpr size_t componentCount(const T& v)
{
	return T::num_components;
}

void getOrthonormalBasis(const osg::Vec3f &normal, osg::Vec3f &tangent, osg::Vec3f &bitangent);

inline glm::vec2 toGlmVec2(const osg::Vec2f& v)
{
	return glm::vec2(v.x(), v.y());
}

inline glm::dvec2 toGlmDvec2(const osg::Vec2d& v)
{
	return glm::dvec2(v.x(), v.y());
}

inline glm::vec3 toGlmVec3(const osg::Vec3f& v)
{
	return glm::vec3(v.x(), v.y(), v.z());
}

inline glm::dvec3 toGlmDvec3(const osg::Vec3d& v)
{
	return glm::dvec3(v.x(), v.y(), v.z());
}

inline glm::dquat toGlmDquat(const osg::Quat& v)
{
	return glm::dquat(v.w(), v.x(), v.y(), v.z());
}

inline osg::Vec2f toOsgVec2f(const glm::vec2& v)
{
	return osg::Vec2f(v.x, v.y);
}

inline osg::Vec2d toOsgVec2d(const glm::dvec2& v)
{
	return osg::Vec2d(v.x, v.y);
}

inline osg::Vec3f toOsgVec3f(const glm::vec3& v)
{
	return osg::Vec3f(v.x, v.y, v.z);
}

inline osg::Vec3d toOsgVec3d(const glm::dvec3& v)
{
	return osg::Vec3d(v.x, v.y, v.z);
}

inline osg::Quat toOsgQuat(const glm::dquat& v)
{
	return osg::Quat(v.x, v.y, v.z, v.w);
}

} // namespace math
} // namespace skybolt
