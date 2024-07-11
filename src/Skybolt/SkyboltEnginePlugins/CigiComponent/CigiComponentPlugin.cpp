/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "CigiComponentPlugin.h"
#include "CigiClient.h"

#include <SkyboltEngine/EngineRoot.h>
#include <SkyboltEngine/Scenario/ScenarioMetadataComponent.h>
#include <SkyboltEngine/Plugin/Plugin.h>
#include <SkyboltSim/SimMath.h>
#include <SkyboltSim/World.h>
#include <SkyboltSim/Components/AttachmentComponent.h>
#include <SkyboltSim/Components/CameraComponent.h>
#include <SkyboltSim/Components/NameComponent.h>
#include <SkyboltSim/Spatial/Geocentric.h>
#include <SkyboltSim/Spatial/GreatCircle.h>
#include <SkyboltSim/Spatial/LatLonAlt.h>
#include <SkyboltSim/Spatial/Orientation.h>
#include <SkyboltCommon/MapUtility.h>

#include <boost/config.hpp>
#include <boost/dll/alias.hpp>
#include <assert.h>

namespace skybolt {
using namespace sim;

class MyCigiEntity : public CigiEntity
{
public:
	MyCigiEntity(const sim::EntityPtr& entity) : mEntity(entity)
	{
		assert(mEntity);
	}

	void setPosition(const sim::LatLonAlt& position) override
	{
		mPosition = position;
		sim::setPosition(*mEntity, llaToGeocentric(position, earthRadius()));
	}

	void setOrientation(const sim::Vector3& rpy) override
	{
		sim::LtpNedOrientation orientation(math::quatFromEuler(rpy));
		sim::Quaternion orientationQuat = sim::toGeocentric(orientation, toLatLon(mPosition)).orientation;
		sim::setOrientation(*mEntity, orientationQuat);
	}

	void setVisible(bool visibile) override
	{
		// TODO
	}

	sim::EntityPtr mEntity;

private:
	sim::LatLonAlt mPosition = sim::LatLonAlt(0,0,0);
};

class MyCigiCamera : public CigiCamera
{
public:
	MyCigiCamera(const sim::World* world, const sim::EntityPtr& entity) :
		mWorld(world),
		mEntity(entity)
	{
		assert(mWorld);
		mCameraComponent = mEntity->getFirstComponent<sim::CameraComponent>();
	}

	void setParent(const CigiEntityPtr& parent) override
	{
		if (mParent != parent)
		{
			if (parent)
			{
				auto parentEntity = static_cast<MyCigiEntity*>(parent.get())->mEntity;

				AttachmentParams params;
				params.positionRelBody = math::dvec3Zero();
				params.orientationRelBody = math::dquatIdentity();

				mAttachmentComponent = std::make_shared<AttachmentComponent>(params, mWorld, mEntity.get());
				mAttachmentComponent->setParentEntityId(parentEntity->getId());
				mEntity->addComponent(mAttachmentComponent);
			}
			else
			{
				mEntity->removeComponent(mAttachmentComponent);
				mAttachmentComponent.reset();
			}
		}
		mParent = parent;
	}

	void setPositionOffset(const sim::Vector3& position) override
	{
		mAttachmentComponent->setPositionRelBody(position);
	}

	void setOrientationOffset(const sim::Vector3& rpy) override
	{
		mAttachmentComponent->setOrientationRelBody(math::quatFromEuler(rpy));
	}

	void setHorizontalFieldOfView(float fov) override
	{
	}

	void setVerticalFieldOfView(float fov) override
	{
		mCameraComponent->getState().fovY = fov;
	}

	const sim::World* mWorld;
	sim::EntityPtr mEntity;
	CigiEntityPtr mParent;
	std::shared_ptr<sim::CameraComponent> mCameraComponent;
	std::shared_ptr<sim::AttachmentComponent> mAttachmentComponent;
};

typedef std::map<int, std::string> TemplatesMap;

class CigiSkyboltWorld : public CigiWorld
{
public:
	CigiSkyboltWorld(EngineRoot* engineRoot, const TemplatesMap& templates, Entity* cigiGatewayEntity) :
		mEngineRoot(engineRoot),
		mTemplates(templates),
		mCigiGatewayEntity(cigiGatewayEntity)
	{
	}

	CigiEntityPtr createEntity(int typeId) override
	{
		auto it = mTemplates.find(typeId);
		if (it != mTemplates.end())
		{
			std::string templateName = it->second;
			sim::EntityPtr entity = mEngineRoot->entityFactory->createEntity(templateName);
			entity->addComponent(createScenarioMetadataComponent());
			entity->setDynamicsEnabled(false);

			mEngineRoot->scenario->world.addEntity(entity);
			return std::make_shared<MyCigiEntity>(entity);
		}
		return nullptr;
	}

	void destroyEntity(const CigiEntityPtr& cigiEntity) override
	{
		sim::EntityPtr simEntity = static_cast<MyCigiEntity*>(cigiEntity.get())->mEntity;
		mEngineRoot->scenario->world.removeEntity(simEntity.get());
	}

	CigiCameraPtr createCamera() override
	{
		sim::EntityPtr entity = mEngineRoot->entityFactory->createEntity("Camera");
		entity->addComponent(createScenarioMetadataComponent());
		mEngineRoot->scenario->world.addEntity(entity);
		return std::make_shared<MyCigiCamera>(&mEngineRoot->scenario->world, entity);
	}

	void destroyCamera(const CigiCameraPtr& cigiCamera) override
	{
		sim::EntityPtr simEntity = static_cast<MyCigiCamera*>(cigiCamera.get())->mEntity;
		mEngineRoot->scenario->world.removeEntity(simEntity.get());
	}

private:
	std::shared_ptr<ScenarioMetadataComponent> createScenarioMetadataComponent() const
	{
		auto metadata = std::make_shared<ScenarioMetadataComponent>();
		metadata->serializable = false;
		metadata->deletable = false;
		metadata->directory = { "Entity", getName(*mCigiGatewayEntity) };
		return metadata;
	}

private:
	EngineRoot* mEngineRoot;
	TemplatesMap mTemplates;
	Entity* mCigiGatewayEntity;
};

typedef std::shared_ptr<CigiClient> CigiClientPtr;
typedef std::shared_ptr<CigiSkyboltWorld> CigiSkyboltWorldPtr;

class CigiComponent : public sim::Component
{
public:
	CigiComponent(const CigiClientPtr& client) :
		mClient(client)
	{
		assert(mClient);
	}

	SKYBOLT_BEGIN_REGISTER_UPDATE_HANDLERS
		SKYBOLT_REGISTER_UPDATE_HANDLER(sim::UpdateStage::Input, update)
	SKYBOLT_END_REGISTER_UPDATE_HANDLERS

	void update()
	{
		mClient->sendFrame();
		mClient->update();
	}

private:
	CigiClientPtr mClient;
};

const std::string cigiComponentName = "cigi";

CigiComponentPlugin::CigiComponentPlugin(const PluginConfig& config) :
	mComponentFactoryRegistry(config.simComponentFactoryRegistry)
{
	EngineRoot* engineRoot = config.engineRoot;

	auto factory = std::make_shared<ComponentFactoryFunctionAdapter>([=](Entity* entity, const ComponentFactoryContext& context, const nlohmann::json& json) {

		TemplatesMap templatesMap;

		CigiClientConfig clientConfig;
		clientConfig.cigiMajorVersion = json.at("cigiMajorVersion").get<int>();
		clientConfig.host = json.at("hostAddress").get<std::string>();
		clientConfig.hostPort = json.at("hostPort").get<int>();
		clientConfig.igPort = json.at("igPort").get<int>();
			 
		auto it = json.find("entityTypes");
		if (it != json.end())
		{
			for (const auto& type : it.value().items())
			{
				int id = std::stoi(type.key());
				templatesMap[id] = type.value().get<std::string>();
			}
		}

		clientConfig.world = std::make_shared<CigiSkyboltWorld>(engineRoot, templatesMap, entity);

		auto communicator = std::make_shared<CigiClient>(clientConfig);

		return std::make_shared<CigiComponent>(communicator);
	});

	mComponentFactoryRegistry->insert(std::make_pair(cigiComponentName, factory));
}

CigiComponentPlugin::~CigiComponentPlugin()
{
	mComponentFactoryRegistry->erase(cigiComponentName);
}

namespace plugins {

	std::shared_ptr<Plugin> createCigiComponentPlugin(const PluginConfig& config)
	{
		return std::make_shared<CigiComponentPlugin>(config);
	}

	BOOST_DLL_ALIAS(
		plugins::createCigiComponentPlugin,
		createEnginePlugin
	)
}

} // namespace skybolt {