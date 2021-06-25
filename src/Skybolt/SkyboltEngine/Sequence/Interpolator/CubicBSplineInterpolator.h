/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "Interpolator.h"
#include <glm/glm.hpp>
#include <functional>

namespace skybolt {

template <typename T>
class CubicBSplineInterpolator : public Interpolator<T>
{
public:
	typedef std::function<int()> ArraySizeGetter;
	typedef std::function<T(int index)> ArrayValueGetter;

	CubicBSplineInterpolator(const ArraySizeGetter& sizeGetter, const ArrayValueGetter& valueGetter, const ArrayValueGetter& timeGetter) :
		mSizeGetter(sizeGetter),
		mValueGetter(valueGetter),
		mTimeGetter(timeGetter)
	{
	}

	//! @param u is the parameteric interpolation coordinate in range [0, 1]
	T interpolate(int firstIndex, int secondIndex, double u) const override 
	{
		glm::dvec4 p;
		p.x = mValueGetter(firstIndex);
		p.y = calcControlPointRight(firstIndex);
		p.z = calcControlPointLeft(secondIndex);
		p.w = mValueGetter(secondIndex);

		// Weights from https://ocw.mit.edu/courses/electrical-engineering-and-computer-science/6-837-computer-graphics-fall-2012/lecture-notes/MIT6_837F12_Lec01.pdf
		double u2 = u * u;
		double u3 = u2 * u;
		double oneMinusU = 1.0 - u;
		double oneMinusU2 = oneMinusU * oneMinusU;
		double oneMinusU3 = oneMinusU2 * oneMinusU;

		glm::dvec4 w;
		w.x = oneMinusU3;
		w.y = 3 * u * oneMinusU2;
		w.z = 3 * u2 * oneMinusU;
		w.w = u3;

		return glm::dot(p, w);
	}

private:
	inline T calcControlPointLeft(int index) const
	{
		return calcControlPoint(index, false);
	}

	inline T calcControlPointRight(int index) const
	{
		return calcControlPoint(index, true);
	}

	T calcControlPoint(int index, bool rightSide) const
	{
		// Calculate gradient
		int size = mSizeGetter();
		int indexPrev = glm::max(0, index - 1);
		int indexNext = glm::min(size - 1, index + 1);

		using vec = glm::tvec2<T>;
		vec p0(mTimeGetter(indexPrev), mValueGetter(indexPrev));
		vec p1(mTimeGetter(index), mValueGetter(index));
		vec p2(mTimeGetter(indexNext), mValueGetter(indexNext));

		vec v0 = glm::normalize(p1 - p0);
		vec v1 = glm::normalize(p2 - p1);

		vec v = glm::normalize(v0 + v1);
		T grad = (v.x > T(0)) ? (v.y / v.x) : T(0);

		// Calculate control point offset along tangent by distance of 1/3 to next point
		T timeOffset = T(1.0/3.0) * (rightSide ? (p2.x - p1.x) : (p0.x - p1.x));
		return p1.y + grad * timeOffset;
	}

private:
	ArraySizeGetter mSizeGetter;
	ArrayValueGetter mValueGetter;
	ArrayValueGetter mTimeGetter;
};

typedef CubicBSplineInterpolator<double> CubicBSplineInterpolatorD;

} // namespace skybolt