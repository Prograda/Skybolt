/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltQt/SkyboltQtFwd.h"
#include <map>
#include <memory>
#include <typeindex>

using ScenarioObjectTypeMap = std::map<std::type_index, ScenarioObjectTypePtr>; //!< Type index is of the class derived from ScenarioObject which this type creates

using ScenarioObjectTypeMapPtr = std::shared_ptr<ScenarioObjectTypeMap>;