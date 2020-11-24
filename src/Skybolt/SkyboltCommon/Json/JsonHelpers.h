/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <nlohmann/json.hpp>
#include <boost/optional.hpp>

namespace skybolt {

template <typename T>
T readOptionalOrDefault(const nlohmann::json& j, const std::string& key, const T& defaultValue)
{
	auto i = j.find(key);
	if (i != j.end())
	{
		return i.value().get<T>();
	}
	return defaultValue;
}

template <typename T>
boost::optional<T> readOptional(const nlohmann::json& j, const std::string& key)
{
	auto i = j.find(key);
	if (i != j.end())
	{
		return i.value().get<T>();
	}
	return boost::none;
}

template <typename T>
static std::map<std::string, T> readNameMap(const nlohmann::json& j, const std::string& key)
{
	std::map<std::string, std::string> result;
	auto item = j.find(key);
	if (item != j.end())
	{
		for (const auto& i : item.value().items())
		{
			result[i.key()] = i.value().get<T>();
		}
	}

	return result;
}

} // namespace skybolt