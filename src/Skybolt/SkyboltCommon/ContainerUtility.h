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

template <typename ContainerT, typename PredicateT>
void eraseIf(ContainerT& items, const PredicateT& predicate) {
	for (auto it = items.begin(); it != items.end(); )
	{
		if (predicate(*it))
		{
			it = items.erase(it);
		}
		else
		{
			++it;
		}
	}
}

template <typename SourceContainerT, typename ResultContainerT, typename Functor>
void transform(const SourceContainerT& c, ResultContainerT& result, Functor &&f)
{
	std::transform(std::begin(c), std::end(c), std::inserter(result, std::end(result)), f);
}

} // namespace skybolt