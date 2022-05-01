/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <SkyboltVis/Window/DisplaySettings.h>
#include <boost/program_options/variables_map.hpp>
#include <nlohmann/json.hpp>

namespace skybolt {

nlohmann::json createDefaultEngineSettings();
nlohmann::json readEngineSettings(const boost::program_options::variables_map& params);

vis::DisplaySettings getDisplaySettingsFromEngineSettings(const nlohmann::json& j);

} // namespace skybolt
