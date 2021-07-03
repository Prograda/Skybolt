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
