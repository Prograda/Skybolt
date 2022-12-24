/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <SkyboltVis/DisplaySettings.h>
#include <SkyboltVis/Renderable/Clouds/CloudRenderingParams.h>
#include <SkyboltVis/Shadow/ShadowParams.h>
#include <boost/program_options/variables_map.hpp>

#include <nlohmann/json.hpp>
#include <optional>

namespace skybolt {

nlohmann::json createDefaultEngineSettings();
nlohmann::json readEngineSettings(const boost::program_options::variables_map& params);

vis::DisplaySettings getDisplaySettingsFromEngineSettings(const nlohmann::json& engineSettings);
std::optional<vis::ShadowParams> getShadowParams(const nlohmann::json& engineSettings);
vis::CloudRenderingParams getCloudRenderingParams(const nlohmann::json& engineSettings);

} // namespace skybolt
