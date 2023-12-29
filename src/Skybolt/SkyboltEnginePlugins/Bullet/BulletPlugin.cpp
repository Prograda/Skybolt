/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "AltitudeProvider.h"
#include "BulletDynamicBodyComponent.h"
#include "BulletSystem.h"
#include "BulletTypeConversion.h"
#include "BulletWheelsComponent.h"
#include "BulletWorld.h"
#include "KinematicBody.h"
#include "DrivetrainComponent.h"
#include "TerrainCollisionShape.h"
#include <SkyboltSim/CollisionGroupMasks.h>
#include <SkyboltSim/Entity.h>
#include <SkyboltSim/Components/ControlInputsComponent.h>
#include <SkyboltSim/Components/Motion.h>
#include <SkyboltSim/Components/Node.h>
#include <SkyboltSim/Components/OceanComponent.h>
#include <SkyboltSim/Components/PlanetComponent.h>
#include <SkyboltSim/JsonHelpers.h>
#include <SkyboltEngine/ComponentFactory.h>
#include <SkyboltEngine/EngineRoot.h>
#include <SkyboltEngine/Plugin/Plugin.h>
#include <SkyboltCommon/VectorUtility.h>
#include <SkyboltCommon/Json/JsonHelpers.h>

#include <boost/config.hpp>
#include <boost/dll/alias.hpp>
#include <assert.h>

namespace skybolt {
using namespace sim;

class AltitudeProviderAdapter : public sim::AltitudeProvider
{
public:
	AltitudeProviderAdapter(const std::shared_ptr<PlanetAltitudeProvider>& provider) :
		mProvider(provider)
	{
		assert(mProvider);
	}

	double get(const sim::LatLon& position) const override
	{
		return mProvider->getAltitude(position).altitude;
	}

	std::shared_ptr<PlanetAltitudeProvider> mProvider;
};

static btCollisionShapePtr loadPlanetCollisionShape(const PlanetComponent& planet, const OceanComponent* ocean = nullptr)
{
	auto compoundShape = std::make_shared<btCompoundShape>();
	if (planet.altitudeProvider)
	{
		double maxEarthRadius = planet.radius + 9000; // TODO: work out a safe maximum terrain altitude bound
		// TODO: delete shape after use
		btCollisionShape* shape = new sim::TerrainCollisionShape(std::make_shared<AltitudeProviderAdapter>(planet.altitudeProvider), planet.radius, maxEarthRadius);
		compoundShape->addChildShape(btTransform::getIdentity(), shape);
	}

	if (ocean)
	{
		// Add sphere to provide collision detection against ocean
		// TODO: delete shape after use
		compoundShape->addChildShape(btTransform::getIdentity(), new btSphereShape(planet.radius));
		return compoundShape;
	}

	return compoundShape;
}

static sim::ComponentPtr loadBulletDynamicBody(BulletWorld& world, Entity* entity, const ComponentFactoryContext& context, const nlohmann::json& json)
{
	double mass = json.at("mass");
	btVector3 velocity(0, 0, 0);
	int collisionGroupMask = CollisionGroupMasks::simBody;
	int collisionFilterMask = ~0;

	// TODO: dispose of shape
	auto shape = std::make_shared<btBoxShape>(toBtVector3(readVector3(json.at("size")) * 0.5));

	btVector3 momentOfInertia = toBtVector3(readOptionalVector3(json, "momentOfInertia"));
	if (momentOfInertia == btVector3(0, 0, 0))
	{
		// Calculate an approx moment of inertia
		shape->calculateLocalInertia(mass, momentOfInertia);
		momentOfInertia *= 0.25;
	}

	auto body = std::make_shared<BulletDynamicBodyComponent>([&] {
		BulletDynamicBodyComponentConfig c;
		c.ownerEntityId = entity->getId();
		c.world = &world;
		c.node = entity->getFirstComponentRequired<sim::Node>().get();
		c.motion = entity->getFirstComponentRequired<sim::Motion>().get();
		c.mass = mass;
		c.momentOfInertia = momentOfInertia;
		c.shape = shape;
		c.velocity = velocity;
		c.collisionGroupMask = collisionGroupMask;
		c.collisionFilterMask = collisionFilterMask;
		return c;
	}());

	body->setCenterOfMass(readOptionalVector3(json, "centerOfMass"));

	return body;
}

const std::string dynamicBodyComponentName = "dynamicBody";
const std::string kinematicBodyComponentName = "kinematicBody";
const std::string planetKinematicBodyComponentName = "planetKinematicBody";
const std::string wheelsComponentName = "wheels";
const std::string drivetrainComponentName = "drivetrain";

class BulletPlugin : public Plugin
{
public:
	BulletPlugin(const PluginConfig& config) :
		mComponentFactoryRegistry(config.simComponentFactoryRegistry),
		mSystemRegistry(config.engineRoot->systemRegistry),
		mBulletWorld(std::make_unique<BulletWorld>())
	{
		(*mComponentFactoryRegistry)[dynamicBodyComponentName] = std::make_shared<ComponentFactoryFunctionAdapter>([this](Entity* entity, const ComponentFactoryContext& context, const nlohmann::json& json) {
			return loadBulletDynamicBody(*mBulletWorld, entity, context, json);
		});

		(*mComponentFactoryRegistry)[kinematicBodyComponentName] = std::make_shared<ComponentFactoryFunctionAdapter>([this](Entity* entity, const ComponentFactoryContext& context, const nlohmann::json& json) {
			auto node = entity->getFirstComponentRequired<Node>().get();
			auto shape = std::make_shared<btBoxShape>(toBtVector3(readVector3(json.at("size")) * 0.5));
			return std::make_shared<KinematicBody>(mBulletWorld.get(), entity->getId(), node, shape, CollisionGroupMasks::simBody);
		});

		(*mComponentFactoryRegistry)[planetKinematicBodyComponentName] = std::make_shared<ComponentFactoryFunctionAdapter>([this](Entity* entity, const ComponentFactoryContext& context, const nlohmann::json& json) {
			auto node = entity->getFirstComponentRequired<Node>().get();
			auto planet = entity->getFirstComponentRequired<PlanetComponent>().get();
			auto ocean = entity->getFirstComponent<OceanComponent>().get();
			btCollisionShapePtr shape = loadPlanetCollisionShape(*planet, ocean);
			return std::make_shared<KinematicBody>(mBulletWorld.get(), entity->getId(), node, shape, CollisionGroupMasks::terrain);
		});

		(*mComponentFactoryRegistry)[wheelsComponentName] = std::make_shared<ComponentFactoryFunctionAdapter>([this](Entity* entity, const ComponentFactoryContext& context, const nlohmann::json& json) {
			auto body = entity->getFirstComponentRequired<BulletDynamicBodyComponent>().get();

			BulletWheelsComponentConfig config;
			config.body = body->getRigidBody();
			config.mass = body->getMass();
			config.world = mBulletWorld->getDynamicsWorld();

			bool enableSteering = false;

			for (const auto& jsonWheel : json.items())
			{
				auto j = jsonWheel.value();
				WheelConfig wheel;
				wheel.attachmentPoint = readVector3(j.at("attachmentPoint"));
				wheel.wheelRadius = j.at("wheelRadius");
				wheel.maxSuspensionTravelLength = j.at("maxSuspensionTravelLength");
				wheel.stiffness = j.at("stiffness");
				wheel.dampingCompression = j.at("dampingCompression");
				wheel.dampingRelaxation = j.at("dampingRelaxation");
				wheel.maxSteeringAngleRadians = readOptionalOrDefault(j, "steeringAngleDeg", 0.0) * math::degToRadD();
				wheel.drivenByEngine = readOptionalOrDefault(j, "drivenByEngine", false);
				config.wheels.push_back(wheel);

				if (wheel.maxSteeringAngleRadians > 0.0)
				{
					enableSteering = true;
				}
			}

			if (enableSteering)
			{
				auto inputsComponent = entity->getFirstComponentRequired<ControlInputsComponent>();
				config.steering = inputsComponent->createOrGet("steering", 0.0f, posNegUnitRange<float>());
			}

			return std::make_shared<BulletWheelsComponent>(config);
		});

		(*mComponentFactoryRegistry)[drivetrainComponentName] = std::make_shared<ComponentFactoryFunctionAdapter>([this](Entity* entity, const ComponentFactoryContext& context, const nlohmann::json& json) {
			auto inputsComponent = entity->getFirstComponentRequired<ControlInputsComponent>();
			auto wheelsComponent = entity->getFirstComponentRequired<BulletWheelsComponent>();

			auto throttle = inputsComponent->createOrGet("throttle", 0.0f, unitRange<float>());
			return std::make_shared<DrivetrainComponent>(wheelsComponent, throttle, json.at("maxForce"));
		});

		mBulletSystem = std::make_shared<BulletSystem>(mBulletWorld.get());
		mSystemRegistry->push_back(mBulletSystem);
	}

	~BulletPlugin()
	{
		eraseFirst(*mSystemRegistry, mBulletSystem);
		mComponentFactoryRegistry->erase(dynamicBodyComponentName);
		mComponentFactoryRegistry->erase(planetKinematicBodyComponentName);
	}

private:
	SystemPtr mBulletSystem;
	std::unique_ptr<BulletWorld> mBulletWorld;
	ComponentFactoryRegistryPtr mComponentFactoryRegistry;
	SystemRegistryPtr mSystemRegistry;
};

namespace plugins {

	std::shared_ptr<Plugin> createBulletPlugin(const PluginConfig& config)
	{
		return std::make_shared<BulletPlugin>(config);
	}

	BOOST_DLL_ALIAS(
		plugins::createBulletPlugin,
		createEnginePlugin
	)
}

} // namespace skybolt