/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "Sprocket/SprocketFwd.h"
#include <map>
#include <typeindex>

using ScenarioObjectTypeMap = std::map<std::type_index, ScenarioObjectTypePtr>; //!< Type index is of the class derived from ScenarioObject which this type creates