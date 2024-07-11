/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <string>

namespace skybolt {

//! Computes the SHA1 of a string
std::string calcSha1(const std::string& p_arg);

} // namespace skybolt