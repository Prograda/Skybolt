/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <optional>
#include <functional>

namespace skybolt {

template <typename T>
T optionalMapOrElse(const std::optional<T>& opt, const std::function<T()> onAbsent)
{
	if (opt)
	{
		return *opt;
	}
	return onAbsent();
}

template <typename T>
void optionalIfPresent(const std::optional<T>& opt, const std::function<void(const T& value)> action)
{
	if (opt)
	{
		return action(*opt);
	}
}

} // namespace skybolt
