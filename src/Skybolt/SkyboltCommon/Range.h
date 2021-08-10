/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <limits>

namespace skybolt {

template <typename T>
struct Range
{
	Range(T first, T last) :
		first(first), last(last) {}

	Range() :
		first(std::numeric_limits<T>::max()),
		last(std::numeric_limits<T>::lowest())
	{
	}

	bool isEmpty() const
	{
		return last < first;
	}

	T first;
	T last;
};

typedef Range<int> IntRange;
typedef Range<double> DoubleRange;

} // namespace skybolt