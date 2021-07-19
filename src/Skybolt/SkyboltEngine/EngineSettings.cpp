/* Copyright 2012-2021 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "EngineSettings.h"
#include "EngineCommandLineParser.h"
#include <SkyboltCommon/OptionalUtility.h>

namespace skybolt {

nlohmann::json createDefaultEngineSettings()
{
	return R"({
	"tileApiKeys": {
		"bing": "",
		"mapbox": ""
	},
	"shadows": {
		"enabled": true
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

} // namespace skybolt
