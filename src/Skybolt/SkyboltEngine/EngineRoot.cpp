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
#include <SkyboltVis/Renderable/Model/ModelFactory.h>
#include <SkyboltVis/Renderable/Planet/Tile/TileSource/JsonTileSourceFactory.h>
#include <SkyboltCommon/File/FileUtility.h>
#include <SkyboltCommon/Math/MathUtility.h>

#define PX_SCHED_IMPLEMENTATION 1
#include <px_sched/px_sched.h>

#include <osgDB/Registry>

#include <boost/log/trivial.hpp>

namespace skybolt {

static void registerAssetModule(const std::string& folderName)
{
	osgDB::Registry::instance()->getDataFilePathList().push_back("Assets/" + folderName + "/");
}

static file::Path locateFile(const std::string& filename, file::FileLocatorMode mode)
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
	vis::ModelFactoryConfig config;
	config.defaultProgram = programs.model;
	for (const std::string& name : transparentMaterialNames())
	{
		config.stateSetModifiers[name] = [=](osg::StateSet& stateSet, const osg::Material& material) {
			stateSet.setAttribute(programs.glass);
			vis::makeStateSetTransparent(stateSet);
		};
	}

	return std::make_shared<vis::ModelFactory>(config);
}

EngineRoot::EngineRoot(const EngineRootConfig& config) :
	mPluginFactories(config.pluginFactories),
	scheduler(new px_sched::Scheduler),
	fileLocator(locateFile),
	simWorld(std::make_unique<sim::World>())
{

	// Create coreCount threads - 1 background threads, leaving a core for the main thread.
	int coreCount = std::thread::hardware_concurrency();
	int threadCount = std::max(1, coreCount-1);
	BOOST_LOG_TRIVIAL(info) << coreCount << " CPU cores detected. Creating " << threadCount << " background threads.";

	px_sched::SchedulerParams schedulerParams;
	schedulerParams.max_running_threads = threadCount;
	schedulerParams.num_threads = threadCount;
	scheduler->init(schedulerParams);

	osgDB::Registry::instance()->getDataFilePathList().push_back("Source/Assets/");
	osgDB::Registry::instance()->getDataFilePathList().push_back("Assets/");

	file::Paths folders = file::findFoldersInDirectory("Assets");
	for (const auto& folder : folders)
	{
		std::string folderName = folder.stem().string();
		mAssetFolderNames.push_back(folderName);
		registerAssetModule(folderName);
		BOOST_LOG_TRIVIAL(info) << "Registered asset module: " << folderName;
	}

	programs = vis::createShaderPrograms();
	scene.reset(new vis::Scene);

	Scenario* scenarioPtr = &scenario;
	julianDateProvider = [=]() {
		return scenarioPtr->startJulianDate + scenarioPtr->timeSource.getTime() / (60.0 * 60.0 * 24.0);
	};

	auto componentFactoryRegistry = std::make_shared<ComponentFactoryRegistry>();
	addDefaultFactories(*componentFactoryRegistry);

	auto visFactoryRegistry = std::make_shared<vis::VisFactoryRegistry>();
	vis::addDefaultFactories(*visFactoryRegistry);

	// Create object factory
	EntityFactory::Context context;
	context.scheduler = scheduler.get();
	context.simWorld = simWorld.get();
	context.componentFactoryRegistry = componentFactoryRegistry;
	context.scene = scene.get();
	context.programs = &programs;
	context.julianDateProvider = julianDateProvider;
	context.namedObjectRegistry = &namedObjectRegistry;
	context.stats = &stats;
	context.visFactoryRegistry = visFactoryRegistry;
	context.tileSourceFactory = std::make_shared<vis::JsonTileSourceFactory>(config.tileSourceFactoryConfig);
	context.modelFactory = createModelFactory(programs);
	context.fileLocator = locateFile;

	file::Paths paths = getFilePathsInAssetFolders(*this, "Entities", ".json");
	entityFactory.reset(new EntityFactory(context, paths));

	// Create default systems
	systemRegistry = std::make_shared<sim::SystemRegistry>(sim::SystemRegistry({
		std::make_shared<sim::EntitySystem>(simWorld.get()),
		std::make_shared<SimVisSystem>(simWorld.get(), scene)
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
}

file::Paths getFilePathsInAssetFolders(const EngineRoot& engineRoot, const std::string& relativePath, const std::string& extension)
{
	file::Paths result;
	auto folderNames = engineRoot.getAssetFolderNames();
	for (const auto& folderName : folderNames)
	{
		std::string path = "Assets/" + folderName + "/" + relativePath;
		if (boost::filesystem::exists(path))
		{
			auto paths = file::findFilenamesInDirectory(path, extension);
			result.insert(result.end(), paths.begin(), paths.end());
		}
	}
	return result;
}

} // namespace skybolt