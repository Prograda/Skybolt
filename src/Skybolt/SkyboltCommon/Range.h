/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <limits>

namespace skybolt {

//! Represents a range [a, b]
template <typename T>
struct RangeInclusive
{
	RangeInclusive(T first, T last) :
		first(first), last(last) {}

	RangeInclusive() :
		first(std::numeric_limits<T>::max()),
		last(std::numeric_limits<T>::lowest())
	{
	}

	bool isEmpty() const
	{
		return last < first;
	}

	union {
		T first;
		T minimum;
	};

	union {
		T last;
		T maximum;
	};
};

//! Represents a range [a, b)
template <typename T>
struct RangeClosedOpen
{
	RangeClosedOpen(T first, T last) :
		first(first), last(last) {}

	RangeClosedOpen() :
		first(std::numeric_limits<T>::max()),
		last(std::numeric_limits<T>::lowest())
	{
	}

	bool isEmpty() const
	{
		return last <= first;
	}

	T first;
	T last;
};

typedef RangeInclusive<int> IntRangeInclusive;
typedef RangeClosedOpen<int> IntRangeClosedOpen;

typedef RangeInclusive<double> DoubleRangeInclusive;
typedef RangeClosedOpen<double> DoubleRangeClosedOpen;

} // namespace skybolt