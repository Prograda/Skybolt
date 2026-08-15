/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "ThirdParty/barkeep.h"

#include <SkyboltCommon/Json/ReadJsonFile.h>
#include <SkyboltEngine/EngineCommandLineParser.h>
#include <SkyboltEngine/EngineRoot.h>
#include <SkyboltEngine/EngineRootFactory.h>
#include <SkyboltEngine/EntityFactory.h>
#include <SkyboltEngine/Scenario/ScenarioSerialization.h>
#include <SkyboltEngine/Plugin/PluginHelpers.h>
#include <SkyboltEngine/UpdateLoop/SimUpdater.h>

#include <boost/program_options.hpp>

#include <atomic>
#include <chrono>
#include <csignal>
#include <iostream>
#include <sstream>
#include <thread>

namespace po = boost::program_options;
using namespace skybolt;

namespace {

std::atomic_bool gStopRequested = false;

extern "C" void handleInterruptSignal(int signal)
{
	if (signal == SIGINT)
	{
		gStopRequested.store(true);
	}
}

class ScopedSignalHandler
{
public:
	ScopedSignalHandler()
		: mPreviousHandler(std::signal(SIGINT, handleInterruptSignal))
	{
		gStopRequested.store(false);
	}

	~ScopedSignalHandler()
	{
		std::signal(SIGINT, mPreviousHandler);
	}

private:
	typedef void(*SignalHandler)(int);
	SignalHandler mPreviousHandler;
};

std::vector<PluginFactory> loadDefaultPluginFactories()
{
	return loadPluginFactories<Plugin, PluginConfig>(getAllPluginFilepathsInDirectories(EngineRootFactory::getDefaultPluginDirs()));
}

std::string createTopLevelHelpText()
{
	std::ostringstream stream;
	stream << "Usage:\n";
	stream << "  skybolt <command>\n\n";
	stream << "Commands:\n";
	stream << "  run\n";
	stream << "  asset\n";
	stream << "  help\n";
	return stream.str();
}

std::string createRunHelpText(const po::options_description& options)
{
	std::ostringstream stream;
	stream << "Usage:\n";
	stream << "  skybolt run <scenarioFilename.json> [options...]\n\n";
	stream << options;
	return stream.str();
}

std::string createAssetHelpText()
{
	std::ostringstream stream;
	stream << "Usage:\n";
	stream << "  skybolt asset <command>\n\n";
	stream << "Commands:\n";
	stream << "  validate\n";
	stream << "  help\n";
	return stream.str();
}

std::string createAssetValidateHelpText(const po::options_description& options)
{
	std::ostringstream stream;
	stream << "Usage:\n";
	stream << "  skybolt asset validate [options...]\n\n";
	stream << options;
	return stream.str();
}

po::options_description createRunOptions()
{
	po::options_description options("Run options");
	options.add_options()
		("real-time", "run simulation in real time")
		("scenario", po::value<std::string>(), "scenario filename")
		("no-tty", "disable TTY in console output");
	EngineCommandLineParser::addOptions(options);
	return options;
}

po::options_description createAssetValidateOptions()
{
	po::options_description options("Asset validate options");
	EngineCommandLineParser::addOptions(options);
	return options;
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

std::vector<std::string> getArgs(int argc, char** argv)
{
	std::vector<std::string> args;
	for (int i = 1; i < argc; ++i)
	{
		args.emplace_back(argv[i]);
	}
	return args;
}

bool isHelpToken(const std::string& arg)
{
	return arg == "help";
}

nlohmann::json readEngineSettingsFromCommandLine(const po::variables_map& vm)
{
	if (auto settings = EngineCommandLineParser::readSettings(vm))
	{
		return *settings;
	}
	return nlohmann::json::object();
}

std::unique_ptr<EngineRoot> createEngineRoot(bool enableVis, const po::variables_map& vm)
{
	EngineRootConfig config;
	config.enableVis = enableVis;
	config.assetSearchPaths = getDefaultAssetSearchPaths();
	config.engineSettings = readEngineSettingsFromCommandLine(vm);

	auto engineRoot = std::make_unique<EngineRoot>(config);
	engineRoot->loadPlugins(loadDefaultPluginFactories());
	return engineRoot;
}

int runScenario(const po::variables_map& vm)
{
	if (!vm.count("scenario"))
	{
		throw std::runtime_error("Missing scenario filename.");
	}

	ScopedSignalHandler signalHandler;
	auto engineRoot = createEngineRoot(false, vm);
	engineRoot->scenario->timeSource->setState(sim::TimeSource::StatePlaying);

	const std::string scenarioFilename = vm["scenario"].as<std::string>();
	const bool realTime = vm.count("real-time") > 0;
	const bool noTty = vm.count("no-tty") > 0;

	try
	{
		const nlohmann::json scenarioJson = readJsonFile(scenarioFilename).at("scenario");

		readScenario(*engineRoot->typeRegistry, *engineRoot->scenario,
			[factory = engineRoot->entityFactory.get()](const std::string& templateName, const std::string& instanceName) {
				return factory->createEntity(templateName, instanceName);
			},
			scenarioJson);
	}
	catch (const std::exception& e)
	{
		throw std::runtime_error("Failed to read scenario '" + scenarioFilename + "': " + std::string(e.what()));
	}

	const auto& timeSource = engineRoot->scenario->timeSource;
	timeSource->setState(sim::TimeSource::StatePlaying);
	SimUpdater updater(engineRoot.get());
	updater.setRequestedTimeRate(realTime ? 1.0 : 1000000.0);

	const sim::SecondsD simDuration = timeSource->getRange().getDuration();

	std::atomic<size_t> elapsedSeconds{0};
	auto bar =
		barkeep::ProgressBar(&elapsedSeconds, {
		  .total = std::size_t(std::ceil(simDuration)),
		  .message = "Simulating",
		  .speed = 1.,
		  .speed_unit = "x real time",
		  .interval = noTty ? 10.0 : 1.0,
		  .no_tty = noTty});
	bar->show();

	auto previousTime = std::chrono::steady_clock::now();
	while (!gStopRequested.load() && timeSource->getTime() + 1e-9 < timeSource->getRange().end)
	{
		auto currentTime = std::chrono::steady_clock::now();
		double wallDt = std::chrono::duration<double>(currentTime - previousTime).count();
		previousTime = currentTime;
		updater.update(wallDt);

		elapsedSeconds = size_t(timeSource->getTime() - timeSource->getRange().start);
	}

	if (gStopRequested.load())
	{
		std::cout << "\nInterrupted." << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

int validateAssets(const po::variables_map& vm)
{
	auto engineRoot = createEngineRoot(true, vm);

	int validatedCount = 0;
	for (const auto& templateName : engineRoot->entityFactory->getTemplateNames())
	{
		auto entity = engineRoot->entityFactory->createEntity(templateName);
		engineRoot->scenario->world.addEntity(entity);
		engineRoot->scenario->world.removeEntity(entity.get());
		++validatedCount;
	}

	std::cout << "Validated " << validatedCount << " entity templates." << std::endl;
	return EXIT_SUCCESS;
}

int handleRunCommand(int argc, char** argv)
{
	if (argc >= 2 && isHelpToken(argv[1]))
	{
		auto options = createRunOptions();
		std::cout << createRunHelpText(options);
		return EXIT_SUCCESS;
	}

	auto options = createRunOptions();
	po::positional_options_description positional;
	positional.add("scenario", 1);

	auto vm = parseCommandLine(argc, argv, options, positional);
	if (vm.count("help"))
	{
		std::cout << createRunHelpText(options);
		return EXIT_SUCCESS;
	}
	return runScenario(vm);
}

int handleAssetValidateCommand(int argc, char** argv)
{
	if (argc >= 2 && isHelpToken(argv[1]))
	{
		auto options = createAssetValidateOptions();
		std::cout << createAssetValidateHelpText(options);
		return EXIT_SUCCESS;
	}

	auto options = createAssetValidateOptions();
	po::positional_options_description positional;
	auto vm = parseCommandLine(argc, argv, options, positional);
	if (vm.count("help"))
	{
		std::cout << createAssetValidateHelpText(options);
		return EXIT_SUCCESS;
	}
	return validateAssets(vm);
}

int handleAssetCommand(const std::vector<std::string>& args, int argc, char** argv)
{
	if (args.size() < 2 || isHelpToken(args[1]))
	{
		std::cout << createAssetHelpText();
		return EXIT_SUCCESS;
	}

	if (args[1] == "validate")
	{
		return handleAssetValidateCommand(argc, argv);
	}

	throw std::runtime_error("Unknown asset command. Run 'skybolt help' for usage.");
}

} // namespace

int main(int argc, char** argv)
{
	try
	{
		auto args = getArgs(argc, argv);
		if (args.empty() || args[0] == "help")
		{
			std::cout << createTopLevelHelpText();
			return EXIT_SUCCESS;
		}

		if (args[0] == "run")
		{
			return handleRunCommand(argc - 1, argv + 1);
		}

		if (args[0] == "asset")
		{
			return handleAssetCommand(args, argc - 2, argv + 2);
		}

		throw std::runtime_error("Unknown command. Run 'skybolt help' for usage.");
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}
	catch (...)
	{
		std::cerr << "Unknown exception" << std::endl;
	}
	return EXIT_FAILURE;
}
