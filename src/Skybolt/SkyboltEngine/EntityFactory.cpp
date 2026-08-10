/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "EntityFactory.h"
#include "AssetPackage.h"
#include "EngineRoot.h"
#include "EngineSettings.h"
#include "Components/PlanetElevationComponent.h"
#include "Components/TemplateNameComponent.h"
#include "Scenario/ScenarioMetadataComponent.h"

#include <SkyboltSim/JsonHelpers.h>
#include <SkyboltSim/World.h>
#include <SkyboltSim/WorldUtil.h>
#include <SkyboltSim/Components/AtmosphereComponent.h>
#include <SkyboltSim/Components/MainRotorComponent.h>
#include <SkyboltSim/Components/NameComponent.h>
#include <SkyboltSim/Components/Node.h>
#include <SkyboltSim/Components/OceanComponent.h>
#include <SkyboltSim/Components/ParticleSystemComponent.h>
#include <SkyboltSim/Components/PlanetComponent.h>
#include <SkyboltSim/Components/PropellerComponent.h>
#include <SkyboltSim/Components/SunComponent.h>
#include <SkyboltSim/Particles/ParticleSystem.h>
#include <SkyboltSim/Physics/Astronomy.h>
#include <SkyboltSim/Spatial/GreatCircle.h>

#include <SkyboltCommon/Random.h>
#include <SkyboltCommon/StringVector.h>
#include <SkyboltCommon/File/FileUtility.h>
#include <SkyboltCommon/Json/JsonHelpers.h>
#include <SkyboltCommon/Json/ReadJsonFile.h>
#include <SkyboltCommon/Math/MathUtility.h>
#include <filesystem>

using namespace skybolt;
using namespace skybolt::sim;

const ScenarioObjectPath& skybolt::getDefaultEntityScenarioObjectDirectory()
{
	static ScenarioObjectPath d = {};
	return d;
}

static std::shared_ptr<ScenarioMetadataComponent> createDefaultEntityScenarioMetadataComponent()
{
	auto component = std::make_shared<ScenarioMetadataComponent>();
	component->directory = getDefaultEntityScenarioObjectDirectory();
	return component;
}

EntityPtr EntityFactory::createEntityFromJson(const nlohmann::json& json, const std::string& templateName, const std::string& instanceName, const Vector3& position, const Quaternion& orientation, EntityId id) const
{
	EntityPtr entity = std::make_shared<sim::Entity>((id != nullEntityId()) ? id : generateNextEntityId());

	// Create required components
	entity->addComponent(std::make_shared<NameComponent>(instanceName));
	entity->addComponent(std::make_shared<TemplateNameComponent>(templateName));

	// Create additional components from json
	ComponentFactoryContext componentFactoryContext{
	.scheduler = mContext.scheduler,
	.simWorld = mContext.simWorld,
	.entityFactory = this,
	.imageFactory = mContext.imageFactory,
	.julianDateProvider = mContext.julianDateProvider,
	.stats = mContext.stats,
	.tileSourceFactoryRegistry = mContext.tileSourceFactoryRegistry,
	.fileLocator = mContext.fileLocator,
	.assetPackagePaths = mContext.assetPackagePaths,
	.engineSettings = mContext.engineSettings,
	.factoryRegistries = mContext.factoryRegistries,
	.typeRegistry = mContext.typeRegistry
	};

	const nlohmann::json& components = json.at("components");
	for (const auto& component : components)
	{
		for (nlohmann::json::const_iterator componentIt = component.begin(); componentIt != component.end(); ++componentIt)
		{
			std::string key = componentIt.key();
			const nlohmann::json& content = componentIt.value();
			
			bool componentKeyHandled = false;

			// Sim components
			{
				auto it = mContext.componentFactoryRegistry->find(key);
				if (it != mContext.componentFactoryRegistry->end())
				{
					componentKeyHandled = true;

					ComponentFactoryPtr factory = it->second;
					auto newComponent = factory->create(entity.get(), componentFactoryContext, content);
					if (newComponent)
					{
						entity->addComponent(newComponent);
					}
					else
					{
						// TODO: some of the component factories re-use existing components that
						// are already on the entity and return nullptr. Therefore if a factory
						// returns nullptr it should not be regarded as an error.
						// This is a confusing design and should be improved.
					}
				}
			}

			if (!componentKeyHandled)
			{
				if (!key.starts_with("_"))
				{
					SKYBOLT_LOG(warning) << "Unknown component key: " << key;
				}
			}
		}
	}

	// Add default ScenarioMetadataComponent if one wasn't in the json file
	auto metadata = entity->getFirstComponent<ScenarioMetadataComponent>();
	if (!metadata)
	{
		metadata = createDefaultEntityScenarioMetadataComponent();
		entity->addComponent(metadata);
	}
	metadata->directory = getScenarioObjectDirectoryForTemplate(templateName);

	// Initialise entity pose
	if (Node* node = entity->getFirstComponent<Node>().get(); node)
	{
		node->setPosition(position);
		node->setOrientation(orientation);
	}

	return entity;
}

static std::optional<ScenarioObjectPath> readScenarioObjectDirectory(const nlohmann::json& json)
{
	std::optional<ScenarioObjectPath> result;
	ifChildExists(json, "components", [&](const nlohmann::json& components) {
		for (const auto& component : components)
		{
			ifChildExists(component, "scenarioMetadata", [&] (const nlohmann::json& c) {
				if (c.contains("scenarioObjectDirectory"))
				{
					result = parseStringList(c.at("scenarioObjectDirectory").get<std::string>(), "/");
				}
			});
		}
	});
	return result;
}

static std::optional<std::string> getEntityName(const std::filesystem::path& filepath)
{
	std::string filename = filepath.filename().string();
	if (filename.ends_with(EntityFactory::entityTemplateFileExtension))
	{
		return std::string(filename.substr(0, filename.size() - EntityFactory::entityTemplateFileExtension.size()));
	}
	return std::nullopt;
}

static std::optional<std::string> getDirectoryRelativeToFolder(const std::filesystem::path& filepath, const std::string& folderInPath)
{
	std::string pathStr = filepath.parent_path().generic_string();
	std::string target = "/" + folderInPath + "/";

	size_t pos = pathStr.find(target);
	if (pos != std::string::npos)
	{
		return pathStr.substr(pos + target.length());
	}

	return std::nullopt;
}

EntityFactory::EntityFactory(const EntityFactory::Context& context, const std::vector<std::filesystem::path>& entityFilenames) :
	mContext(context)
{
	assert(context.julianDateProvider);
	assert(context.programs);
	assert(context.simWorld);
	assert(context.stats);
	assert(context.tileSourceFactoryRegistry);
	assert(context.scene);

	for (const std::filesystem::path& filename : entityFilenames)
	{
		auto name = getEntityName(filename.string());
		if (!name)
		{
			SKYBOLT_LOG(error) << "Entity filename has invalid extension: " << filename;
			continue;
		}

		nlohmann::json json = readJsonFile(filename.string());
		mTemplateJsonMap[*name] = json;
		mTemplateNames.push_back(*name);

		std::optional<ScenarioObjectPath> directory = readScenarioObjectDirectory(json);
		if (!directory)
		{
			if (auto path = getDirectoryRelativeToFolder(filename, "Entities"); path)
			{
				directory = parseStringList(*path, "/");

				// If the directory is the same as the template name, remove it from the path
				if (directory->back() == name)
				{
					directory->pop_back();
				}
			}
		}
		mTemplateDirectories[*name] = directory.value_or(getDefaultEntityScenarioObjectDirectory());

	}
}

EntityPtr EntityFactory::createEntity(const std::string& templateName, const std::string& instanceName, const Vector3& position, const Quaternion& orientation, EntityId id) const
{
	{
		auto i = mTemplateJsonMap.find(templateName);
		if (i != mTemplateJsonMap.end())
		{
			// Construct unique instance name
			std::string uniqueInstanceName = instanceName.empty()
				? createUniqueEntityName(templateName)
				: (isEntityNameUnique(instanceName) ? instanceName : createUniqueEntityName(instanceName));

			try
			{
				return createEntityFromJson(i->second, templateName, uniqueInstanceName, position, orientation, id);
			}
			catch (const std::exception& e)
			{
				throw Exception("Error loading '" + templateName + "': " + e.what());
			}
		}
	}

	throw std::runtime_error("Invalid templateName: " + templateName);
}

const skybolt::ScenarioObjectPath& EntityFactory::getScenarioObjectDirectoryForTemplate(const std::string& templateName) const
{
	if (auto i = mTemplateDirectories.find(templateName); i != mTemplateDirectories.end())
	{
		return i->second;
	}
	return getDefaultEntityScenarioObjectDirectory();
}

std::string EntityFactory::createUniqueEntityName(const std::string& baseName) const
{
	for (int i = 1; i < INT_MAX; ++i)
	{
		std::string name = baseName + std::to_string(i);
		if (isEntityNameUnique(name))
		{
			return name;
		}
	}
	throw skybolt::Exception("Could not create unique object name from base name: " + baseName);
}

bool EntityFactory::isEntityNameUnique(const std::string& name) const
{
	return mContext.simWorld->findObjectByName(name) == nullptr;
}

sim::EntityId EntityFactory::generateNextEntityId() const
{
	++mNextEntityId.entityId;
	return mNextEntityId;
}