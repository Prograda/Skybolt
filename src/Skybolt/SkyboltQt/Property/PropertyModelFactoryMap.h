/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltQt/SkyboltQtFwd.h"
#include <functional>
#include <map>
#include <typeindex>

using PropertyModelFactory = std::function<PropertiesModelPtr(const ScenarioObject&)>;
using PropertyModelFactoryMap = std::map<std::type_index, PropertyModelFactory>;