/* Copyright Matthew Reid
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
#include <SkyboltCommon/File/OsDirectories.h>
#include <SkyboltCommon/Json/JsonHelpers.h>
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

Expected<file::Path> locateFile(const std::string& filename)
{
	auto resolvedFilename = osgDB::Registry::instance()->findDataFile(filename, nullptr, osgDB::CASE_SENSITIVE);
	if (resolvedFilename.empty())
	{
		return UnexpectedMessage{ "Could not locate file: " + filename };
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

const std::string skyboltCacheDirEnvironmentVariable = "SKYBOLT_CACHE_DIR";

static file::Path getDefaultCacheDir()
{
	return file::getAppUserDataDirectory("Skybolt") / "Cache";
}

static file::Path getCacheDir()
{
	if (const char* dir = std::getenv(skyboltCacheDirEnvironmentVariable.c_str()); dir)
	{
		file::Path path(dir);
		if (std::filesystem::exists(path))
		{
			return dir;
		}
		BOOST_LOG_TRIVIAL(error) << "Environment variable '" << skyboltCacheDirEnvironmentVariable << "' not set to a valid path. Using default location: " << getDefaultCacheDir().string();
	}
	return getDefaultCacheDir();
}

EngineRoot::EngineRoot(const EngineRootConfig& config) :
	scheduler(new px_sched::Scheduler),
	fileLocator(locateFile),
	scenario(std::make_unique<Scenario>()),
	typeRegistry(std::make_unique<refl::TypeRegistry>()),
	factoryRegistries(std::make_unique<FactoryRegistries>()),
	engineSettings(config.engineSettings)
{
	int threadCount = determineThreadCountFromHardwareAndUserLimits();

	px_sched::SchedulerParams schedulerParams;
	schedulerParams.max_running_threads = threadCount;
	schedulerParams.num_threads = threadCount;
	scheduler->init(schedulerParams);

	std::vector<std::string> assetSearchPaths = {
		"Assets/",
		"../Assets/"
	};

	if (const char* path = std::getenv("SKYBOLT_ASSETS_PATH"); path)
	{
		auto paths = file::splitByPathListSeparator(std::string(path));
		assetSearchPaths.insert(assetSearchPaths.begin(), paths.begin(), paths.end());
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

	if (config.enableVis)
	{
		programs = vis::createShaderPrograms();
	}
	scene.reset(new vis::Scene(new osg::StateSet()));

	auto julianDateProvider = [scenario = scenario.get()]() {
		return getCurrentJulianDate(*scenario);
	};

	auto componentFactoryRegistry = std::make_shared<ComponentFactoryRegistry>();
	addDefaultFactories(*componentFactoryRegistry);
	factoryRegistries->addItem(componentFactoryRegistry);

	auto visFactoryRegistry = std::make_shared<vis::VisFactoryRegistry>();
	vis::addDefaultFactories(*visFactoryRegistry);
	factoryRegistries->addItem(visFactoryRegistry);

	tileSourceFactoryRegistry = std::make_shared<vis::JsonTileSourceFactoryRegistry>([&] {
		file::Path cacheDir = getCacheDir();
		BOOST_LOG_TRIVIAL(info) << "Using cache directory '" << cacheDir.string() << "'.";
		vis::JsonTileSourceFactoryRegistryConfig c;
		c.apiKeys = readNameMap<std::string>(config.engineSettings, "tileApiKeys");
		c.cacheDirectory = cacheDir.string();
		return c;
	}());
	vis::addDefaultFactories(*tileSourceFactoryRegistry);

	// Create object factory
	EntityFactory::Context context;
	context.scheduler = scheduler.get();
	context.simWorld = &scenario->world;
	context.componentFactoryRegistry = componentFactoryRegistry;
	context.julianDateProvider = julianDateProvider;
	context.stats = &stats;
	context.tileSourceFactoryRegistry = tileSourceFactoryRegistry;
	context.fileLocator = locateFile;
	context.assetPackagePaths = mAssetPackagePaths;
	context.engineSettings = engineSettings;

	if (config.enableVis)
	{
		context.visContext = [&] {
			EntityFactory::VisContext c;
			c.scene = scene.get();
			c.programs = &programs;
			c.visFactoryRegistry = visFactoryRegistry;
			c.modelFactory = createModelFactory(programs);
			c.textureCache = std::make_shared<vis::TextureCache>();
			return c;
		}();
	}

	file::Paths paths = getFilesWithExtensionInDirectoryInAssetPackages(mAssetPackagePaths, "Entities", ".json");
	entityFactory.reset(new EntityFactory(context, paths));

	// Create default systems
	systemRegistry = std::make_shared<sim::SystemRegistry>(sim::SystemRegistry({
		std::make_shared<sim::EntitySystem>(&scenario->world),
		std::make_shared<SimVisSystem>(&scenario->world, scene)
	}));
}

EngineRoot::~EngineRoot()
{
	// shutdown all systems first
	systemRegistry->clear();
}

void EngineRoot::loadPlugins(const std::vector<PluginFactory>& pluginFactories)
{
	PluginConfig config;
	config.engineRoot = this;

	// Create plugins and store them in EngineRoot to ensure plugins exist for lifetime of EngineRoot.
	for (const auto& factory : pluginFactories)
	{
		if (PluginPtr plugin = factory(config); plugin)
		{
			mPlugins.push_back(plugin);
		}
	}

	// We need to store the factories as well to ensure the plugin symbols do not get unloaded.
	mPluginFactories = pluginFactories;
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