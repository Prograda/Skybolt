/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "EngineRoot.h"
#include "AssetPackage.h"
#include "ComponentFactory.h"
#include "SimVisBinding/SimVisSystem.h"
#include <SkyboltSim/CameraController/CameraModifierStack.h>
#include <SkyboltSim/System/EntitySystem.h>
#include <SkyboltSim/System/SimStepper.h>
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
#include <SkyboltCommon/Logging/Logging.h>
#include <optional>

namespace skybolt {

Expected<file::Path> locateFile(const std::string& filename)
{
	auto resolvedFilename = osgDB::Registry::instance()->findDataFile(filename, nullptr, osgDB::CASE_SENSITIVE);
	if (resolvedFilename.empty())
	{
		return UnexpectedMessage{ "Could not locate file: " + filename };
	}
	return resolvedFilename;
};

static std::vector<std::string> transparentMaterialNames() { return { "transparentExt", "transparent" }; } //@deprecated. Set osg::Material diffuse alpha < 1 instead to treat geometry as transparent.

static vis::ModelFactoryPtr createModelFactory(const vis::ShaderPrograms& programs)
{
	osg::ref_ptr<osg::Program> glassProgram = programs.getRequiredProgram("glass");

	vis::ModelFactoryConfig config;
	config.defaultProgram = programs.getRequiredProgram("model");
	config.glassProgram = glassProgram;
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
			SKYBOLT_LOG(error) << maxCoresEnvironmentVariable << " environment variable set to '" << value << "' which is not a valid integer and will be ignored";
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
	SKYBOLT_LOG(info) << coreCount << " CPU cores detected. " << coreLimitMessage << " Creating " << threadCount << " background threads.";
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
		SKYBOLT_LOG(error) << "Environment variable '" << skyboltCacheDirEnvironmentVariable << "' not set to a valid path. Using default location: " << getDefaultCacheDir().string();
	}
	return getDefaultCacheDir();
}

std::vector<std::string> getDefaultAssetSearchPaths()
{
	std::vector<std::string> assetSearchPaths = {
		"Assets/",
		"../Assets/"
	};

	if (const char* path = std::getenv("SKYBOLT_ASSETS_PATH"); path)
	{
		auto paths = file::splitByPathListSeparator(std::string(path));
		assetSearchPaths.insert(assetSearchPaths.begin(), paths.begin(), paths.end());
	}
	return assetSearchPaths;
}

EngineRoot::EngineRoot(const EngineRootConfig& config) :
	scheduler(new px_sched::Scheduler),
	fileLocator(locateFile),
	systemRegistry(std::make_shared<sim::SystemRegistry>()),
	typeRegistry(std::make_unique<refl::TypeRegistry>()),
	factoryRegistries(std::make_unique<FactoryRegistries>()),
	engineSettings(config.engineSettings)
{
	auto simStepper = std::make_unique<sim::SimStepper>(systemRegistry, sim::TimeRange(0, 100));
	scenario = std::make_unique<Scenario>(std::move(simStepper));

	int threadCount = determineThreadCountFromHardwareAndUserLimits();

	px_sched::SchedulerParams schedulerParams;
	schedulerParams.max_running_threads = threadCount;
	schedulerParams.num_threads = threadCount;
	scheduler->init(schedulerParams);
	
	// Load all asset packages
	AssetPackageFilepaths assetPackageFilepaths;
	for (const auto& assetSearchPath : config.assetSearchPaths)
	{
		AssetPackageFilepaths newFilepaths = loadAssetPackagesInPath(assetSearchPath);
		assetPackageFilepaths.insert(newFilepaths.begin(), newFilepaths.end());
	}

	// Ensure the required asset packages were found
	std::set<std::string> missingRequiredPackages = {"Core", "Globe"};
	for (const auto& [name, path] : assetPackageFilepaths)
	{
		missingRequiredPackages.erase(name);
		mAssetPackagePaths.push_back(path.string());
	}

	if (!missingRequiredPackages.empty())
	{
		std::string packagesString = boost::algorithm::join(missingRequiredPackages, ", ");
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

	factoryRegistries->addItem(std::make_shared<sim::CameraModifierFactoryRegistry>());

	tileSourceFactoryRegistry = std::make_shared<vis::JsonTileSourceFactoryRegistry>([&] {
		file::Path cacheDir = getCacheDir();
		SKYBOLT_LOG(info) << "Using cache directory '" << cacheDir.string() << "'.";
		vis::JsonTileSourceFactoryRegistryConfig c;
		c.apiKeys = readNameMap<std::string>(config.engineSettings, "tileApiKeys");
		c.cacheDirectory = cacheDir.string();
		return c;
	}());
	vis::addDefaultFactories(*tileSourceFactoryRegistry);

	// Create entity factory
	EntityFactory::Context context{
	.scheduler = scheduler.get(),
	.simWorld = &scenario->world,
	.julianDateProvider = julianDateProvider,
	.componentFactoryRegistry = componentFactoryRegistry,
	.tileSourceFactoryRegistry = tileSourceFactoryRegistry,
	.stats = &stats,
	.fileLocator = locateFile,
	.assetPackagePaths = mAssetPackagePaths,
	.engineSettings = engineSettings,
	.factoryRegistries = factoryRegistries.get(),
	.typeRegistry = typeRegistry.get()
	};

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

	file::Paths paths = getFilesWithExtensionInDirectoryInAssetPackagesRecursive(mAssetPackagePaths, "Entities", std::string(EntityFactory::entityTemplateFileExtension));
	entityFactory.reset(new EntityFactory(context, paths));

	// Create default systems
	systemRegistry->push_back(std::make_shared<sim::EntitySystem>(&scenario->world));
	systemRegistry->push_back(std::make_shared<SimVisSystem>(&scenario->world, scene));
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
		try
		{
			if (PluginPtr plugin = factory(config); plugin)
			{
				assert(plugin);
				mPlugins.push_back(plugin);
			}
		}
		catch(const std::exception& e)
		{
			SKYBOLT_LOG(error) << "Error loading plugin: " << e.what();
		}
	}

	// We need to store the factories as well to ensure the plugin symbols do not get unloaded.
	mPluginFactories = pluginFactories;
}

} // namespace skybolt