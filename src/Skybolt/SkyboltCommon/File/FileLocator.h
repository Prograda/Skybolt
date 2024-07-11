/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "FileUtility.h"

#include <functional>
#include <string>

namespace skybolt {
namespace file {

enum FileLocatorMode {
	Required,
	Optional
};

using FileLocator = std::function<Path(const std::string& filename, FileLocatorMode)>;

} // namespace file
} // namespace skybolt
