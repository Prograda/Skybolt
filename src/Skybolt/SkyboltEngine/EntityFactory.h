/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "ComponentFactory.h"
#include "SkyboltEngineFwd.h"
#include <SkyboltSim/EntityId.h>
#include <SkyboltSim/SimMath.h>
#include <SkyboltVis/VisFactory.h>
#include <SkyboltVis/SkyboltVisFwd.h>
#include <SkyboltCommon/File/FileLocator.h>
#include <SkyboltCommon/Math/MathUtility.h>

#include <nlohmann/json.hpp>

#include <functional>
#include <map>
#include <vector>

namespace skybolt {

class EntityFactory
{
public:
	struct VisContext
	{
		vis::Scene* scene;
		vis::VisFactoryRegistryPtr visFactoryRegistry;
		const vis::ShaderPrograms* programs;
		vis::ModelFactoryPtr modelFactory;
		vis::TextureCachePtr textureCache;
	};

	struct Context
	{
		px_sched::Scheduler* scheduler;
		sim::World* simWorld;
		JulianDateProvider julianDateProvider;
		ComponentFactoryRegistryPtr componentFactoryRegistry;
		vis::JsonTileSourceFactoryRegistryPtr tileSourceFactoryRegistry;
		EngineStats* stats;
		file::FileLocator fileLocator;
		std::vector<std::string> assetPackagePaths;
		nlohmann::json engineSettings;
		std::optional<VisContext> visContext; // !< If empty, visual objects will not be created
	};

	EntityFactory(const Context& context, const std::vector<std::filesystem::path>& entityFilenames);

	sim::EntityPtr createEntity(const std::string& templateName, const std::string& instanceName = "", const sim::Vector3& position = math::dvec3Zero(), const sim::Quaternion& orientation = math::dquatIdentity(), sim::EntityId id = sim::nullEntityId()) const;
	sim::EntityPtr createEntityFromJson(const nlohmann::json& json, const std::string& instanceName, const sim::Vector3& position, const sim::Quaternion& orientation, sim::EntityId id = sim::nullEntityId()) const;

	typedef std::vector<std::string> Strings;
	Strings getTemplateNames() const {return mTemplateNames;}

	std::string createUniqueObjectName(const std::string& baseName) const;

	sim::EntityId generateNextEntityId() const;

private:
	sim::EntityPtr createSun(const EntityFactory::VisContext& visContext) const;
	sim::EntityPtr createMoon(const EntityFactory::VisContext& visContext) const;
	sim::EntityPtr createStars(const EntityFactory::VisContext& visContext) const;

private:
	Strings mTemplateNames;
	std::map<std::string, std::function<sim::EntityPtr()>> mBuiltinTemplates; // TODO: genericize these

	typedef std::map<std::string, nlohmann::json> TemplateJsonMap;
	TemplateJsonMap mTemplateJsonMap;

	Context mContext;
	mutable sim::EntityId mNextEntityId{1,0};
};

} // namespace skybolt