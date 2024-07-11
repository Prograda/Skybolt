/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <string>
#include <vector>

namespace skybolt {

typedef std::vector<std::string> StringVector;
inline std::string toDelimitedString(const StringVector& vec)
{
	if (vec.empty())
	{
		return "";
	}

	std::string r = vec.front();
	for (size_t i = 1; i < vec.size(); ++i)
	{
		r += ", " + vec[i];
	}
	return r;
}

inline StringVector parseStringList(const std::string& str, const std::string& delimeters = ",")
{
	StringVector vec;
	boost::split(vec, str, boost::is_any_of(delimeters), boost::token_compress_on);
	for (std::string& item : vec)
	{
		boost::trim(item);
	}
	return vec;
}

template <typename T>
std::vector<T> parseStringListAndConvert(const std::string& str)
{
	auto values = parseStringList(str);
	std::vector<T> result;
	result.reserve(values.size());
	for (const auto& value : values)
	{
		result.push_back(boost::lexical_cast<T>(value));
	}
	return result;
}

} // namespace skybolt
