/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "ComponentFactory.h"
#include "SkyboltEngineFwd.h"
#include "SkyboltEngine/Scenario/ScenarioObjectPath.h"
#include <SkyboltSim/EntityId.h>
#include <SkyboltSim/SimMath.h>
#include <SkyboltVis/VisFactory.h>
#include <SkyboltVis/SkyboltVisFwd.h>
#include <SkyboltCommon/File/FileLocator.h>
#include <SkyboltCommon/Math/MathUtility.h>

#include <nlohmann/json.hpp>

#include <functional>
#include <map>
#include <optional>
#include <vector>

namespace skybolt {

class EntityFactoryBase
{
public:
	virtual ~EntityFactoryBase() = default;
	virtual sim::EntityPtr createEntity(const std::string& templateName, const std::string& instanceName = "", const sim::Vector3& positionn = math::dvec3Zero(), const sim::Quaternion& orientation = math::dquatIdentity(), sim::EntityId id = sim::nullEntityId()) const = 0;

	typedef std::map<std::string, nlohmann::json> TemplateJsonMap;
	virtual const TemplateJsonMap& getTemplateJsonMap() const = 0;
};

class EntityFactory : public EntityFactoryBase
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
		vis::JsonTileSourceFactoryRegistryPtr tileSourceFactoryRegistry; //!< Never null
		EngineStats* stats;
		file::FileLocator fileLocator;
		std::vector<std::string> assetPackagePaths;
		nlohmann::json engineSettings;
		std::optional<VisContext> visContext; // !< If empty, visual objects will not be created
		NonNullPtr<FactoryRegistries> factoryRegistries;
		NonNullPtr<refl::TypeRegistry> typeRegistry;
	};

	EntityFactory(const Context& context, const std::vector<std::filesystem::path>& entityFilenames);
	~EntityFactory() override = default;

	sim::EntityPtr createEntity(const std::string& templateName, const std::string& instanceName = "", const sim::Vector3& position = math::dvec3Zero(), const sim::Quaternion& orientation = math::dquatIdentity(), sim::EntityId id = sim::nullEntityId()) const override;
	sim::EntityPtr createEntityFromJson(const nlohmann::json& json, const std::string& templateName, const std::string& instanceName, const sim::Vector3& position, const sim::Quaternion& orientation, sim::EntityId id = sim::nullEntityId()) const;

	typedef std::vector<std::string> Strings;
	Strings getTemplateNames() const { return mTemplateNames; }

	 //!< Gets the default scenario object directory for objects created from the given template
	const skybolt::ScenarioObjectPath& getScenarioObjectDirectoryForTemplate(const std::string& templateName) const;

	typedef std::map<std::string, nlohmann::json> TemplateJsonMap;
	const TemplateJsonMap& getTemplateJsonMap() const override { return mTemplateJsonMap; }

	std::string createUniqueEntityName(const std::string& baseName) const;

	bool isEntityNameUnique(const std::string& name) const;

	sim::EntityId generateNextEntityId() const;

	constexpr static std::string_view entityTemplateFileExtension = ".entity.json";

private:
	Strings mTemplateNames;
	std::map<std::string, skybolt::ScenarioObjectPath> mTemplateDirectories;

	TemplateJsonMap mTemplateJsonMap;

	Context mContext;
	mutable sim::EntityId mNextEntityId{1,0};
};

const ScenarioObjectPath& getDefaultEntityScenarioObjectDirectory();

} // namespace skybolt