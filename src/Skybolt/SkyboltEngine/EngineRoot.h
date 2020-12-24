/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "EngineStats.h"
#include "Scenario.h"
#include "EntityFactory.h"
#include "Plugin/Plugin.h"
#include <SkyboltSim/Components/NameComponent.h>
#include <SkyboltSim/System/SystemRegistry.h>
#include <SkyboltVis/Scene.h>
#include <SkyboltVis/ShaderProgramRegistry.h>
#include <SkyboltVis/Renderable/Planet/Tile/TileSource/JsonTileSourceFactory.h>
#include <SkyboltCommon/File/FileUtility.h>

#include <memory>

namespace skybolt {

typedef std::function<PluginPtr(const PluginConfig&)> PluginFactory;

struct EngineRootConfig
{
	std::vector<PluginFactory> pluginFactories;
	vis::JsonTileSourceFactoryConfig tileSourceFactoryConfig;
};

class EngineRoot
{
private:
	// This private block is first because these objects should be disposed of last
	std::vector<PluginFactory> mPluginFactories;
	std::vector<PluginPtr> mPlugins;
	std::vector<std::string> mAssetFolderNames;

public:
	EngineRoot(const EngineRootConfig& config);
	~EngineRoot();

	const std::vector<std::string>& getAssetFolderNames() const { return mAssetFolderNames; }

	vis::ShaderPrograms programs;
	vis::ScenePtr scene;
	std::unique_ptr<px_sched::Scheduler> scheduler;
	file::FileLocator fileLocator;
	JulianDateProvider julianDateProvider;
	std::unique_ptr<sim::World> simWorld;
	std::unique_ptr<EntityFactory> entityFactory;
	sim::NamedObjectRegistry namedObjectRegistry;
	EngineStats stats;
	Scenario scenario;
	sim::SystemRegistryPtr systemRegistry;
};

//! @param relativePath is a path, relative to the asset folder name, in which to search for files
file::Paths getFilePathsInAssetFolders(const EngineRoot& engineRoot, const std::string& relativePath, const std::string& extension);

} // namespace skybolt