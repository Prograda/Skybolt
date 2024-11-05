/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "PythonInterpreter.h"
#include "PythonPluginUtil.h"
#include <SkyboltEngine/EngineRoot.h>
#include <SkyboltEngine/Plugin/Plugin.h>

#include <fstream>
#include <istream>
#include <boost/config.hpp>
#include <boost/dll/alias.hpp>

namespace skybolt {

using namespace sim;

const std::string pythonComponentName = "python";

static std::string removeWhitespace(const std::string& input)
{
    std::string result = input;
    result.erase(std::remove_if(result.begin(), result.end(), ::isspace), result.end());
    return result;
}

static bool isPythonPluginModule(const file::Path& filename)
{
	std::ifstream file(filename);
	if (!file.is_open())
	{
		return false;
	}

	// Return true if the file contains a line with `def skybolt_register():`, ignoring whitespace
	std::string line;
	while (std::getline(file, line))
	{
		if (removeWhitespace(line).find("defskybolt_register():") != std::string::npos)
		{
			return true;
		}
	}

	return false;
}

class PythonComponentPlugin : public Plugin
{
public:
	PythonComponentPlugin(const PluginConfig& config) :
		mPythonInterpreter(config.engineRoot)
	{
		file::Paths moduleFilenames = getFilesWithExtensionInDirectoryInAssetPackages(config.engineRoot->getAssetPackagePaths(), "Scripts", ".py");
		std::vector<std::string> moduleNames;
		for (const file::Path& moduleFilename : moduleFilenames)
		{
			if (isPythonPluginModule(moduleFilename))
			{
				moduleNames.push_back(moduleFilename.stem().string());
			}
		}

		try
		{
			loadPythonPluginModules(moduleNames);
		}
		catch (const std::exception& e)
		{
			BOOST_LOG_TRIVIAL(error) << e.what();
		}
	}

	~PythonComponentPlugin() override = default;

private:
	PythonInterpreter mPythonInterpreter;
};

namespace plugins {

	std::shared_ptr<Plugin> createEnginePlugin(const PluginConfig& config)
	{
		return std::make_shared<PythonComponentPlugin>(config);
	}

	BOOST_DLL_ALIAS(
		plugins::createEnginePlugin,
		createEnginePlugin
	)
}

} // namespace skybolt