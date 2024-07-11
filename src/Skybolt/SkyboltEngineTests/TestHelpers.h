/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <catch2/catch.hpp>
#include <osg/Vec3d>

inline void check(const osg::Vec2d& a, const osg::Vec2d& b, double eps)
{
	CHECK(a.x() == Approx(b.x()).margin(eps));
	CHECK(a.y() == Approx(b.y()).margin(eps));
}

inline void check(const osg::Vec3d& a, const osg::Vec3d& b, double eps)
{
	CHECK(a.x() == Approx(b.x()).margin(eps));
	CHECK(a.y() == Approx(b.y()).margin(eps));
	CHECK(a.z() == Approx(b.z()).margin(eps));
}
