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

} // namespace skybolt