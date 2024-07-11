/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <stdexcept>

namespace skybolt {

struct Exception : public std::runtime_error
{
	Exception(const std::string& message) : std::runtime_error(message) {}
};

} //namespace skybolt