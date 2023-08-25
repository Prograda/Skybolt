/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#pragma once

#include <vector>
#include <algorithm>
#include <assert.h>

namespace skybolt {

template <typename T>
static const T* findFirst(const std::vector<T>& v, const T& t)
{
	auto it = std::find(v.begin(), v.end(), t);
	if (it != v.end())
	{
		return &*it;
	}
	return nullptr;
}

template <typename T>
static bool contains(const std::vector<T>& v, const T& t)
{
	return std::find(v.begin(), v.end(), t) != v.end();
}

//! @returns true if item was erased
template <typename T>
static bool eraseFirst(std::vector<T>& v, const T& t)
{
	typename std::vector<T>::iterator it = std::find(v.begin(), v.end(), t);
	if (it != v.end())
	{
		v.erase(it);
		return true;
	}
	return false;
}

//! Erases element at index by overwriting it with a copy of the last element
//! and popping the last element. The order of the vector is not preserved.
template <typename T>
static void fastUnstableEraseAtIndex(std::vector<T>& v, size_t index)
{
	assert(index < v.size());

	if (v.size() == 1)
	{
		v.clear();
		return;
	}

	// Copy last item to index
	size_t last = v.size() - 1;
	v[index] = v[last];
	v.pop_back();
}

} // namespace skybolt