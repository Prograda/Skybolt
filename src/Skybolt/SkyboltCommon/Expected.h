/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "Exception.h"

#include <optional>
#include <string>
#include <variant>
#include <boost/log/trivial.hpp>

namespace skybolt {

struct UnexpectedMessage { std::string str; };

//! This could be replaced by std::expected (c++23)
template <typename T>
using Expected = std::variant<T, UnexpectedMessage>;

template <typename T>
std::optional<T> value(const Expected<T>& expected)
{
	if (std::holds_alternative<T>(expected))
	{
		return std::get<T>(expected);
	}

	return std::nullopt;
}

template <typename T, typename FunctionT>
std::optional<T> valueOrElse(const Expected<T>& expected, const FunctionT& f)
{
	static_assert(std::is_convertible<FunctionT, std::function<void(const UnexpectedMessage&)>>::value, "Function must take UnexpectedMessage as an argument");

	if (std::holds_alternative<T>(expected))
	{
		return std::get<T>(expected);
	}

	f(std::get<UnexpectedMessage>(expected));
	return std::nullopt;
}

template <typename T>
std::optional<T> valueOrLogWarning(const Expected<T>& expected)
{
	return valueOrElse(expected, [](const auto& m) {
		BOOST_LOG_TRIVIAL(warning) << m.str;
		});
}

template <typename T>
std::optional<T> valueOrLogError(const Expected<T>& expected)
{
	return valueOrElse(expected, [](const auto& m) {
		BOOST_LOG_TRIVIAL(error) << m.str;
		});
}

template <typename T, class ExceptionT>
T valueOrThrow(const Expected<T>& expected)
{
	if (std::holds_alternative<T>(expected))
	{
		return std::get<T>(expected);
	}
	throw ExceptionT(std::get<UnexpectedMessage>(expected).str);
}

template <typename T>
T valueOrThrowException(const Expected<T>& expected)
{
	return valueOrThrow<T, Exception>(expected);
}

} // namespace skybolt