/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "AltitudeProvider.h"
#include "BulletDynamicBodyComponent.h"
#include "BulletTypeConversion.h"
#include "BulletWheelsComponent.h"
#include "BulletWorld.h"
#include "KinematicBody.h"
#include "DrivetrainComponent.h"
#include "TerrainCollisionShape.h"
#include <SkyboltSim/CollisionGroupMasks.h>
#include <SkyboltSim/Entity.h>
#include <SkyboltSim/Components/ControlInputsComponent.h>
#include <SkyboltSim/Components/Node.h>
#include <SkyboltSim/Components/PlanetComponent.h>
#include <SkyboltSim/JsonHelpers.h>
#include <SkyboltSim/System/System.h>
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
	AltitudeProviderAdapter(const std::shared_ptr<AsyncPlanetAltitudeProvider>& provider) :
		mProvider(provider)
	{
		assert(mProvider);
	}

	double get(const sim::LatLon& position) const override
	{
		auto altitude = mProvider->getAltitudeOrRequestLoad(position);
		if (!altitude)
		{
			return 0.0; // TODO: what fallback to use here?
		}
		return *altitude;
	}

	std::shared_ptr<AsyncPlanetAltitudeProvider> mProvider;
};

static btCollisionShape* loadPlanetCollisionShape(const PlanetComponent& planet)
{
	// TODO: dispose of the shapes
	double maxEarthRadius = planet.radius + 9000; // TODO: work out a safe maximum terrain altitude bound
	btCollisionShape* shape = new sim::TerrainCollisionShape(std::make_shared<AltitudeProviderAdapter>(planet.altitudeProvider), planet.radius, maxEarthRadius);

	if (planet.hasOcean)
	{
		// Add sphere to provide collision detection against ocean
		btCompoundShape* compoundShape = new btCompoundShape();
		compoundShape->addChildShape(btTransform::getIdentity(), new btSphereShape(planet.radius));
		compoundShape->addChildShape(btTransform::getIdentity(), shape);
		return compoundShape;
	}

	return shape;
}

static sim::ComponentPtr loadBulletDynamicBody(BulletWorld& world, Entity* entity, const ComponentFactoryContext& context, const nlohmann::json& json)
{
	Real mass = json.at("mass");
	btVector3 velocity(0, 0, 0);
	int collisionGroupMask = CollisionGroupMasks::simBody;
	int collisionFilterMask = ~0;

	// TODO: dispose of shape
	btCollisionShape* shape = new btBoxShape(toBtVector3(readVector3(json.at("size")) * 0.5));

	btVector3 momentOfInertia = toBtVector3(readOptionalVector3(json, "momentOfInertia"));
	if (momentOfInertia == btVector3(0, 0, 0))
	{
		// Calculate an approx moment of inertia
		shape->calculateLocalInertia(mass, momentOfInertia);
		momentOfInertia *= 0.25;
	}

	auto body = std::make_shared<BulletDynamicBodyComponent>(&world, entity->getFirstComponentRequired<sim::Node>().get(), mass, momentOfInertia, shape,
		velocity, collisionGroupMask, collisionFilterMask);

	body->setCenterOfMass(readOptionalVector3(json, "centerOfMass"));

	return body;
}

class BulletSystem : public System
{
public:
	BulletSystem(BulletWorld* world) :
		mWorld(world)
	{
		assert(mWorld);
	}

	void updateDynamicsSubstep(double dtSubstep) override
	{
		mWorld->getDynamicsWorld()->stepSimulation(dtSubstep, 0, dtSubstep);
	};

private:
	BulletWorld* mWorld;
};

const std::string dynamicBodyComponentName = "dynamicBody";
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

		(*mComponentFactoryRegistry)[planetKinematicBodyComponentName] = std::make_shared<ComponentFactoryFunctionAdapter>([this](Entity* entity, const ComponentFactoryContext& context, const nlohmann::json& json) {
			auto node = entity->getFirstComponentRequired<Node>().get();
			auto planet = entity->getFirstComponentRequired<PlanetComponent>().get();
			btCollisionShape* shape = loadPlanetCollisionShape(*planet);
			return std::make_shared<KinematicBody>(mBulletWorld.get(), node, shape, CollisionGroupMasks::terrain);
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
		VectorUtility::eraseFirst(*mSystemRegistry, mBulletSystem);
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

	std::shared_ptr<Plugin> createEnginePlugin(const PluginConfig& config)
	{
		return std::make_shared<BulletPlugin>(config);
	}

	BOOST_DLL_ALIAS(
		plugins::createEnginePlugin,
		createEnginePlugin
	)
}

} // namespace skybolt {