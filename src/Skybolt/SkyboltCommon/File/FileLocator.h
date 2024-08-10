/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltCommon/Expected.h"
#include "FileUtility.h"

#include <functional>
#include <string>

namespace skybolt {
namespace file {

//! @param filename is the name of the file to locate
//! @returns full path to located file
using FileLocator = std::function<Expected<Path>(const std::string& filename)>;

} // namespace file
} // namespace skybolt
