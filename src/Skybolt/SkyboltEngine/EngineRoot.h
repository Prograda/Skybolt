/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "EngineStats.h"
#include "EntityFactory.h"
#include "Scenario/Scenario.h"
#include "Plugin/Plugin.h"
#include <SkyboltReflection/SkyboltReflectionFwd.h>
#include <SkyboltVis/Shader/ShaderProgramRegistry.h>
#include <SkyboltVis/Renderable/Planet/Tile/TileSource/JsonTileSourceFactory.h>
#include <SkyboltCommon/File/FileUtility.h>

#include <memory>

namespace skybolt {

typedef std::function<PluginPtr(const PluginConfig&)> PluginFactory;

struct EngineRootConfig
{
	std::vector<PluginFactory> pluginFactories;
	nlohmann::json engineSettings;
	bool enableVis = true; //!< True if the visual subsystem is enabled
};

class EngineRoot
{
private:
	// This private block is first because these objects should be disposed of last
	std::vector<PluginFactory> mPluginFactories;
	std::vector<PluginPtr> mPlugins;
	std::vector<std::string> mAssetPackagePaths;

public:
	EngineRoot(const EngineRootConfig& config);
	~EngineRoot();

	const std::vector<std::string>& getAssetPackagePaths() const { return mAssetPackagePaths; }

	std::unique_ptr<px_sched::Scheduler> scheduler;
	vis::ShaderPrograms programs;
	vis::ScenePtr scene;
	file::FileLocator fileLocator;
	std::unique_ptr<EntityFactory> entityFactory;
	vis::JsonTileSourceFactoryRegistryPtr tileSourceFactoryRegistry;
	EngineStats stats;
	std::unique_ptr<Scenario> scenario;
	sim::SystemRegistryPtr systemRegistry;
	std::unique_ptr<refl::TypeRegistry> typeRegistry;
	nlohmann::json engineSettings;
};

file::Path locateFile(const std::string& filename, file::FileLocatorMode mode);
file::Paths getPathsInAssetPackages(const std::vector<std::string>& assetPackagePaths, const std::string& relativePath);
file::Paths getFilesWithExtensionInDirectoryInAssetPackages(const std::vector<std::string>& assetPackagePaths, const std::string& relativeDirectory, const std::string& extension);

} // namespace skybolt