/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <nlohmann/json.hpp>
#include <optional>

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
std::optional<T> readOptional(const nlohmann::json& j, const std::string& key)
{
	auto i = j.find(key);
	if (i != j.end())
	{
		return i.value().get<T>();
	}
	return std::nullopt;
}

template <typename T>
bool readOptionalToVar(const nlohmann::json& j, const std::string& key, T& var)
{
	auto i = j.find(key);
	if (i != j.end())
	{
		var = i.value().get<T>();
		return true;
	}
	return false;
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

template <typename T>
static std::vector<T> readOptionalVector(const nlohmann::json& j, const std::string& name, const std::vector<T>& defaultValue)
{
	auto item = j.find(name);
	if (item != j.end())
	{
		std::vector<T> r;
		for (const auto& v : item.value())
		{
			r.push_back(v.get<T>());
		}
		return r;
	}
	return defaultValue;
}

template <typename ActionT>
static void ifChildExists(const nlohmann::json& j, const std::string& name, const ActionT& action)
{
	auto item = j.find(name);
	if (item != j.end())
	{
		action(item.value());
	}
}

} // namespace skybolt