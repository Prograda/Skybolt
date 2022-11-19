/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <optional>
#include <map>

namespace skybolt {

template <typename KeyT, typename ValueT>
std::optional<ValueT> findOptional(const std::map<KeyT, ValueT>& m, const KeyT& key)
{
	auto i = m.find(key);
	if (i != m.end())
	{
		return i->second;
	}
	return std::nullopt;
}

template <typename ContainerT, typename PredicateT>
void removeIf(ContainerT& items, const PredicateT& predicate) {
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

} // namespace skybolt
