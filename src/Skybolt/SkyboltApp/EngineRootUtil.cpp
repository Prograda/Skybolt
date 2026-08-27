#include "EngineRootUtil.h"

#include <SkyboltEngine/EngineRoot.h>
#include <SkyboltEngine/EngineRootFactory.h>
#include <SkyboltEngine/EngineCommandLineParser.h>
#include <SkyboltEngine/Plugin/PluginHelpers.h>

using namespace skybolt;

namespace po = boost::program_options;

std::vector<file::Path> loadDefaultPluginFilepaths()
{
	return getAllPluginFilepathsInDirectories(EngineRootFactory::getDefaultPluginDirs());
}

std::vector<PluginFactory> loadDefaultPluginFactories()
{
	return loadPluginFactories<Plugin, PluginConfig>(loadDefaultPluginFilepaths());
}

nlohmann::json readEngineSettingsFromCommandLine(const po::variables_map& vm)
{
	if (auto settings = EngineCommandLineParser::readSettings(vm))
	{
		return *settings;
	}
	return nlohmann::json::object();
}

std::unique_ptr<EngineRoot> createEngineRootWithoutPlugins(const po::variables_map& vm)
{
	EngineRootConfig config;
	config.assetSearchPaths = getDefaultAssetSearchPaths();
	config.engineSettings = readEngineSettingsFromCommandLine(vm);
	config.imageFactory = EngineRootFactory::createDefaultImageFactory();
	return std::make_unique<EngineRoot>(config);
}

std::unique_ptr<EngineRoot> createEngineRoot(const po::variables_map& vm)
{
	auto engineRoot = createEngineRootWithoutPlugins(vm);
	engineRoot->loadPlugins(loadDefaultPluginFactories());
	return engineRoot;
}

po::variables_map parseCommandLine(int argc, char** argv, const po::options_description& options, const po::positional_options_description& positional)
{
	po::variables_map vm;
	auto parsed = po::command_line_parser(argc, argv)
		.options(options)
		.positional(positional)
		.run();
	po::store(parsed, vm);
	po::notify(vm);
	return vm;
}

bool isHelpToken(const std::string& arg)
{
	return arg == "help";
}