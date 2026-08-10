/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "AssetPackage.h"
#include "EngineStats.h"
#include "EntityFactory.h"
#include "FactoryRegistries.h"
#include "Scenario/Scenario.h"
#include "Plugin/Plugin.h"
#include <SkyboltVis/Renderable/Planet/Tile/TileSource/JsonTileSourceFactory.h>
#include <SkyboltCommon/File/FileUtility.h>

#include <memory>

namespace skybolt {

typedef std::function<PluginPtr(const PluginConfig&)> PluginFactory;

struct EngineRootConfig
{
	nlohmann::json engineSettings;
	std::vector<std::string> assetSearchPaths;
	vis::ImageFactoryPtr imageFactory; //!< Never null
};

std::vector<std::string> getDefaultAssetSearchPaths();

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

	void loadPlugins(const std::vector<PluginFactory>& pluginFactories);

	const std::vector<std::string>& getAssetPackagePaths() const { return mAssetPackagePaths; }

	std::unique_ptr<px_sched::Scheduler> scheduler; //!< Never null
	file::FileLocator fileLocator;
	ComponentFactoryRegistryPtr componentFactoryRegistry; //!< Never null
	std::unique_ptr<EntityFactory> entityFactory; //!< Never null
	vis::JsonTileSourceFactoryRegistryPtr tileSourceFactoryRegistry; //!< Never null
	EngineStats stats;
	std::unique_ptr<Scenario> scenario; //!< Never null
	sim::SystemRegistryPtr systemRegistry; //!< Never null
	std::unique_ptr<refl::TypeRegistry> typeRegistry; //!< Never null
	std::unique_ptr<FactoryRegistries> factoryRegistries; //!< Never null
	nlohmann::json engineSettings;
};

Expected<file::Path> locateFile(AssetPackageRegistry& registry, const std::string& filename);

} // namespace skybolt