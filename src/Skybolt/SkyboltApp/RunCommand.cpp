#include "RunCommand.h"
#include "EngineRootUtil.h"
#include "ThirdParty/barkeep.h"

#include <SkyboltCommon/Json/ReadJsonFile.h>
#include <SkyboltSim/System/TimeSource.h>
#include <SkyboltEngine/EngineCommandLineParser.h>
#include <SkyboltEngine/EngineRoot.h>
#include <SkyboltEngine/Scenario/ScenarioSerialization.h>
#include <SkyboltEngine/UpdateLoop/SimUpdater.h>

#include <csignal>
#include <iostream>

using namespace skybolt;
namespace po = boost::program_options;

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

}

int runScenario(const po::variables_map& vm)
{
	if (!vm.count("scenario"))
	{
		throw std::runtime_error("Missing scenario filename.");
	}

	ScopedSignalHandler signalHandler;
	auto engineRoot = createEngineRoot(vm);
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
		throw std::runtime_error("Failed to read scenario '" + scenarioFilename + "' Reason: " + std::string(e.what()));
	}

	try
	{
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
	}
	catch (const std::exception& e)
	{
		// Coppy and rethrow the exception to avoid it being destroyed when the engineRoot is destroyed.
		throw std::runtime_error("Error running scenario: " + std::string(e.what()));
	}

	if (gStopRequested.load())
	{
		std::cout << "\nInterrupted." << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

std::string createRunHelpText(const po::options_description& options)
{
	std::ostringstream stream;
	stream << "Usage:\n";
	stream << "  skybolt run <scenarioFilename.json> [options...]\n\n";
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