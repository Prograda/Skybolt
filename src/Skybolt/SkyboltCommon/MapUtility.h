/* Copyright Matthew Reid
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

template <typename ContainerT, typename KeyT>
static const typename ContainerT::value_type::second_type* findValuePtr(const ContainerT& c, const KeyT& k)
{
	if (auto i = c.find(k); i != c.end())
	{
		return &i->second;
	}
	return nullptr;
}

template <typename ContainerT, typename KeyT>
static typename ContainerT::value_type::second_type* findValuePtr(ContainerT& c, const KeyT& k)
{
	if (auto i = c.find(k); i != c.end())
	{
		return &i->second;
	}
	return nullptr;
}

template <typename KeyT, typename ValueT>
std::vector<ValueT> toValuesVector(const std::map<KeyT, ValueT>& m)
{
	std::vector<ValueT> r;
	for (const auto& i : m)
	{
		r.push_back(i.second);
	}
	return r;
}

} // namespace skybolt
