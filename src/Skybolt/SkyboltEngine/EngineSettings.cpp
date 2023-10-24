/* Copyright 2012-2021 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "EngineSettings.h"
#include "EngineCommandLineParser.h"
#include <SkyboltCommon/OptionalUtility.h>
#include <SkyboltCommon/Json/JsonHelpers.h>

namespace skybolt {

nlohmann::json createDefaultEngineSettings()
{
	return R"({
	"tileApiKeys": {
		"bing": "",
		"mapbox": ""
	},
	"display": {
		"multiSampleCount": 4,
		"texturePoolSizeMB": 512,
		"vsync": true
	},
	"mouse": {
		"sensitivity": 1.0
	},
	"shadows": {
		"enabled": true,
		"textureSize": 2048,
		"cascadeBoundingDistances": [0.02, 20.0, 70.0, 250.0, 7000]
	},
	"clouds": {
		"enableTemporalUpscaling": true
	}
})"_json;
}

nlohmann::json readEngineSettings(const boost::program_options::variables_map& params)
{
	nlohmann::json settings = createDefaultEngineSettings();
	skybolt::optionalIfPresent<nlohmann::json>(EngineCommandLineParser::readSettings(params), [&](const nlohmann::json& newSettings) {
		settings.update(newSettings);
	});
	return settings;
}

vis::DisplaySettings getDisplaySettingsFromEngineSettings(const nlohmann::json& engineSettings)
{
	vis::DisplaySettings s {};
	if (const auto& it = engineSettings.find("display"); it != engineSettings.end())
	{
		const auto& j = it.value();
		readOptionalToVar(j, "multiSampleCount", s.multiSampleCount);
		if (readOptionalToVar(j, "texturePoolSizeMB", s.texturePoolSizeBytes))
		{
			s.texturePoolSizeBytes *= 1024 * 1024;
		}
		readOptionalToVar(j, "vsync", s.vsync);
	}
	return s;
}

std::optional<vis::ShadowParams> getShadowParams(const nlohmann::json& engineSettings)
{
	auto i = engineSettings.find("shadows");
	if (i != engineSettings.end())
	{
		if (readOptionalOrDefault<bool>(i.value(), "enabled", true))
		{
			vis::ShadowParams params;
			params.cascadeBoundingDistances = readOptionalVector<float>(i.value(), "cascadeBoundingDistances", {0, 50, 200, 600, 2000});
			params.textureSize = readOptionalOrDefault<int>(i.value(), "textureSize", 1024);

			return params;
		}
	}
	return {};
}

vis::CloudRenderingParams getCloudRenderingParams(const nlohmann::json& engineSettings)
{
	vis::CloudRenderingParams params;
	params.enableTemporalUpscaling = true;

	auto i = engineSettings.find("clouds");
	if (i != engineSettings.end())
	{
		params.enableTemporalUpscaling = readOptionalOrDefault<bool>(i.value(), "enableTemporalUpscaling", true);
	}
	return params;
}

} // namespace skybolt
