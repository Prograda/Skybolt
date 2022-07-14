/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "ComponentFactory.h"
#include "SkyboltEngineFwd.h"
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
	struct Context
	{
		px_sched::Scheduler* scheduler;
		sim::World* simWorld;
		vis::Scene* scene;
		vis::VisFactoryRegistryPtr visFactoryRegistry;
		const vis::ShaderPrograms* programs;
		JulianDateProvider julianDateProvider;
		sim::NamedObjectRegistryPtr namedObjectRegistry;
		ComponentFactoryRegistryPtr componentFactoryRegistry;
		vis::JsonTileSourceFactoryRegistryPtr tileSourceFactoryRegistry;
		vis::ModelFactoryPtr modelFactory;
		EngineStats* stats;
		file::FileLocator fileLocator;
		std::vector<std::string> assetPackagePaths;
		vis::TextureCachePtr textureCache;
		nlohmann::json engineSettings;
	};

	EntityFactory(const Context& context, const std::vector<std::filesystem::path>& entityFilenames);

	sim::EntityPtr createEntity(const std::string& templateName, const std::string& instanceName = "", const sim::Vector3& position = math::dvec3Zero(), const sim::Quaternion& orientation = math::dquatIdentity()) const;
	sim::EntityPtr createEntityFromJson(const nlohmann::json& json, const std::string& instanceName, const sim::Vector3& position, const sim::Quaternion& orientation) const;

	typedef std::vector<std::string> Strings;
	Strings getTemplateNames() const {return mTemplateNames;}

	std::string createUniqueObjectName(const std::string& baseName) const;

private:

	sim::EntityPtr createSun() const;
	sim::EntityPtr createMoon() const;
	sim::EntityPtr createStars() const;
	sim::EntityPtr createPolyline() const;

private:
	Strings mTemplateNames;
	std::map<std::string, std::function<sim::EntityPtr()>> mBuiltinTemplates; // TODO: genericize these

	typedef std::map<std::string, nlohmann::json> TemplateJsonMap;
	TemplateJsonMap mTemplateJsonMap;

	Context mContext;
};

} // namespace skybolt