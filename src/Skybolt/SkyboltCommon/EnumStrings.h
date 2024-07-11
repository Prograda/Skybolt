/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "StringVector.h"
#include <algorithm>

template<typename T> const skybolt::StringVector& skyboltGetEnumStrings()
{
	static skybolt::StringVector emptyVector = {};
	return emptyVector;
}

#define DEFINE_ENUM_STRINGS(EnumType, ...) \
template<> inline const skybolt::StringVector& skyboltGetEnumStrings<EnumType>() \
{ \
	static skybolt::StringVector v = {__VA_ARGS__}; \
	return v; \
} \
inline std::string skyboltEnumToString(EnumType value) \
{ \
	const skybolt::StringVector& v = skyboltGetEnumStrings<EnumType>(); \
	return v[std::max(0, std::min((int)v.size(), (int)value))]; \
}