/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "EngineRoot.h"
#include "ComponentFactory.h"
#include "SimVisBinding/SimVisSystem.h"
#include <SkyboltSim/System/EntitySystem.h>
#include <SkyboltSim/World.h>
#include <SkyboltVis/OsgStateSetHelpers.h>
#include <SkyboltVis/Scene.h>
#include <SkyboltVis/TextureCache.h>
#include <SkyboltVis/Renderable/Model/ModelFactory.h>
#include <SkyboltVis/Renderable/Planet/Tile/TileSource/JsonTileSourceFactory.h>
#include <SkyboltCommon/File/FileUtility.h>
#include <SkyboltCommon/Math/MathUtility.h>

#define PX_SCHED_IMPLEMENTATION 1
#include <px_sched/px_sched.h>

#include <osgDB/Registry>
#include <boost/algorithm/string.hpp>
#include <boost/log/trivial.hpp>
#include <optional>

namespace skybolt {

static void registerAssetPackage(const std::string& folderPath)
{
	osgDB::Registry::instance()->getDataFilePathList().push_back(folderPath + "/");
}

file::Path locateFile(const std::string& filename, file::FileLocatorMode mode)
{
	auto resolvedFilename = osgDB::Registry::instance()->findDataFile(filename, nullptr, osgDB::CASE_SENSITIVE);
	if (resolvedFilename.empty())
	{
		if (mode == file::FileLocatorMode::Required)
		{
			BOOST_LOG_TRIVIAL(error) << "Could not locate file: " << filename;
		}
		return file::Path();
	}
	return resolvedFilename;
};

static std::vector<std::string> transparentMaterialNames() { return { "transparentExt", "transparent" }; }

static vis::ModelFactoryPtr createModelFactory(const vis::ShaderPrograms& programs)
{
	osg::ref_ptr<osg::Program> glassProgram = programs.getRequiredProgram("glass");

	vis::ModelFactoryConfig config;
	config.defaultProgram = programs.getRequiredProgram("model");
	for (const std::string& name : transparentMaterialNames())
	{
		config.stateSetModifiers[name] = [=](osg::StateSet& stateSet, const osg::Material& material) {
			stateSet.setAttribute(glassProgram);
			vis::makeStateSetTransparent(stateSet, vis::TransparencyMode::PremultipliedAlpha);
		};
	}

	return std::make_shared<vis::ModelFactory>(config);
}

const std::string maxCoresEnvironmentVariable = "SKYBOLT_MAX_CORES";

static std::optional<int> getMaxUsableCores()
{
	const char* value = std::getenv(maxCoresEnvironmentVariable.c_str());
	if (value)
	{
		try
		{
			return std::stoi(value);
		}
		catch (const std::invalid_argument&)
		{
			BOOST_LOG_TRIVIAL(error) << maxCoresEnvironmentVariable << " environment variable set to '" << value << "' which is not a valid integer and will be ignored";
		}
	}
	return std::nullopt;
}

static int determineThreadCountFromHardwareAndUserLimits()
{
	// Create coreCount threads - 1 background threads, leaving a core for the main thread.
	int coreCount = std::thread::hardware_concurrency();

	// Apply user configured limit to number of cores
	std::optional<int> maxUsableCores = getMaxUsableCores();
	std::string coreLimitMessage;
	if (maxUsableCores)
	{
		coreCount = std::min(coreCount, *maxUsableCores);
		coreLimitMessage = "Usable cores limited to environment variable " + maxCoresEnvironmentVariable + "=" + std::to_string(coreCount) + ".";
	}

	int threadCount = std::max(1, coreCount - 1);
	BOOST_LOG_TRIVIAL(info) << coreCount << " CPU cores detected. " << coreLimitMessage << " Creating " << threadCount << " background threads.";
	return threadCount;
}

EngineRoot::EngineRoot(const EngineRootConfig& config) :
	mPluginFactories(config.pluginFactories),
	scheduler(new px_sched::Scheduler),
	fileLocator(locateFile),
	scenario(std::make_unique<Scenario>()),
	engineSettings(config.engineSettings)
{
	int threadCount = determineThreadCountFromHardwareAndUserLimits();

	px_sched::SchedulerParams schedulerParams;
	schedulerParams.max_running_threads = threadCount;
	schedulerParams.num_threads = threadCount;
	scheduler->init(schedulerParams);

	std::vector<std::string> assetSearchPaths = {
		"Assets/"
	};

	if (const char* path = std::getenv("SKYBOLT_ASSETS_PATH"); path)
	{
		auto paths = file::splitByPathListSeparator(std::string(path));
		assetSearchPaths.insert(assetSearchPaths.begin(), paths.begin(), paths.end());
	}
	else
	{
		BOOST_LOG_TRIVIAL(warning) << "SKYBOLT_ASSETS_PATH environment variable not set. Skybolt may not be able to find assets. "
			"Please refer to Skybolt documentation for information about setting this variable.";
	}

	std::set<std::string> requiredPackages = {"Core", "Globe"};
	for (const auto& assetSearchPath : assetSearchPaths)
	{
		file::Paths folders = file::findFoldersInDirectory(assetSearchPath);
		for (const auto& folder : folders)
		{
			std::string folderName = folder.stem().string();
			mAssetPackagePaths.push_back(folder.string());
			registerAssetPackage(folder.string());
			BOOST_LOG_TRIVIAL(info) << "Registered asset package: " << folderName;

			requiredPackages.erase(folderName);
		}
	}

	if (!requiredPackages.empty())
	{
		std::string packagesString = boost::algorithm::join(requiredPackages, ", ");
		throw std::runtime_error("Could not find asset packages: {" + packagesString + "}. Ensure working directory and/or SKYBOLT_ASSETS_PATH is set correctly. "
			"Please refer to Skybolt documentation for information about finding assets.");
	}

	programs = vis::createShaderPrograms();
	scene.reset(new vis::Scene(new osg::StateSet()));

	auto julianDateProvider = [scenario = scenario.get()]() {
		return getCurrentJulianDate(*scenario);
	};

	auto componentFactoryRegistry = std::make_shared<ComponentFactoryRegistry>();
	addDefaultFactories(*componentFactoryRegistry);

	auto visFactoryRegistry = std::make_shared<vis::VisFactoryRegistry>();
	vis::addDefaultFactories(*visFactoryRegistry);

	tileSourceFactoryRegistry = std::make_shared<vis::JsonTileSourceFactoryRegistry>(config.tileSourceFactoryRegistryConfig);
	vis::addDefaultFactories(*tileSourceFactoryRegistry);

	// Create object factory
	EntityFactory::Context context;
	context.scheduler = scheduler.get();
	context.simWorld = &scenario->world;
	context.componentFactoryRegistry = componentFactoryRegistry;
	context.scene = scene.get();
	context.programs = &programs;
	context.julianDateProvider = julianDateProvider;
	context.stats = &stats;
	context.visFactoryRegistry = visFactoryRegistry;
	context.tileSourceFactoryRegistry = tileSourceFactoryRegistry;
	context.modelFactory = createModelFactory(programs);
	context.fileLocator = locateFile;
	context.assetPackagePaths = mAssetPackagePaths;
	context.textureCache = std::make_shared<vis::TextureCache>();
	context.engineSettings = engineSettings;

	file::Paths paths = getFilesWithExtensionInDirectoryInAssetPackages(mAssetPackagePaths, "Entities", ".json");
	entityFactory.reset(new EntityFactory(context, paths));

	// Create default systems
	systemRegistry = std::make_shared<sim::SystemRegistry>(sim::SystemRegistry({
		std::make_shared<sim::EntitySystem>(&scenario->world),
		std::make_shared<SimVisSystem>(&scenario->world, scene)
	}));

	// Create plugins
	// TODO: move plugin creation out of constructor. It should happen after EngineRoot is gauranteed to be fully created
	// because plugins may refer to EngineRoot.
	{
		PluginConfig config;
		config.engineRoot = this;
		config.simComponentFactoryRegistry = componentFactoryRegistry;
		config.visFactoryRegistry = visFactoryRegistry;
		for (const auto& factory : mPluginFactories)
		{
			mPlugins.push_back(factory(config));
		}
	}
}

EngineRoot::~EngineRoot()
{
	// shutdown all systems first
	systemRegistry->clear();
}

file::Paths getPathsInAssetPackages(const std::vector<std::string>& assetPackagePaths, const std::string& relativePath)
{
	file::Paths result;
	for (const auto& packagePath : assetPackagePaths)
	{
		std::string path = packagePath + "/" + relativePath;
		if (std::filesystem::exists(path))
		{
			result.push_back(path);
		}
	}
	return result;
}

file::Paths getFilesWithExtensionInDirectoryInAssetPackages(const std::vector<std::string>& assetPackagePaths, const std::string& relativeDirectory, const std::string& extension)
{
	file::Paths result;
	for (const auto& packagePath : assetPackagePaths)
	{
		std::string path = packagePath + "/" + relativeDirectory;
		if (std::filesystem::exists(path))
		{
			auto paths = file::findFilenamesInDirectory(path, extension);
			result.insert(result.end(), paths.begin(), paths.end());
		}
	}
	return result;
}

} // namespace skybolt