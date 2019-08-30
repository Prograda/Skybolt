/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <osg/Vec3>
#include <osg/Vec4>

template <class T>
T componentWiseMultiply(const T& a, const T& b)
{
	return T(a.x() * b.x(), a.y() * b.y());
}

inline osg::Vec3f componentWiseMultiply(const osg::Vec3f& a, const osg::Vec3f& b)
{
	return osg::Vec3f(a.x() * b.x(), a.y() * b.y(), a.z() * b.z());
}

inline osg::Vec4f componentWiseMultiply(const osg::Vec4f& a, const osg::Vec4f& b)
{
	return osg::Vec4f(a.x() * b.x(), a.y() * b.y(), a.z() * b.z(), a.w() * b.w());
}

template <class T>
T componentWiseDivide(const T& a, const T& b)
{
	return T(a.x() / b.x(), a.y() / b.y());
}

template <class T>
inline T componentWiseLerp(const T& a, const T& b, const T& t)
{
	return a + componentWiseMultiply(t, (b - a));
}

template <class T>
T swapComponentsVec2(const T& v)
{
	return T(v.y(), v.x());
}
