/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <SkyboltCommon/NumericComparison.h>

#include <catch2/catch.hpp>
#include <osg/Vec3d>
#include <osg/Vec4d>

inline bool almostEqual(const osg::Vec3f& a, const osg::Vec3f& b, double eps)
{
	if (!skybolt::almostEqual(a.x(), b.x(), eps))
		return false;
	if (!skybolt::almostEqual(a.y(), b.y(), eps))
		return false;
	if (!skybolt::almostEqual(a.z(), b.z(), eps))
		return false;
	return true;
}

inline bool almostEqual(const osg::Vec4f& a, const osg::Vec4f& b, double eps)
{
	if (!skybolt::almostEqual(a.x(), b.x(), eps))
		return false;
	if (!skybolt::almostEqual(a.y(), b.y(), eps))
		return false;
	if (!skybolt::almostEqual(a.z(), b.z(), eps))
		return false;
	if (!skybolt::almostEqual(a.w(), b.w(), eps))
		return false;
	return true;
}