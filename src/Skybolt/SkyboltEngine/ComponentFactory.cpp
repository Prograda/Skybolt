/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "ComponentFactory.h"

#include <SkyboltCommon/StringVector.h>
#include <SkyboltCommon/Json/JsonHelpers.h>
#include <SkyboltEngine/Components/PlanetElevationComponent.h>
#include <SkyboltEngine/Scenario/ScenarioMetadataComponent.h>
#include <SkyboltSim/CollisionGroupMasks.h>
#include <SkyboltSim/JsonHelpers.h>
#include <SkyboltSim/CameraController/AttachedCameraController.h>
#include <SkyboltSim/CameraController/FreeCameraController.h>
#include <SkyboltSim/CameraController/OrbitCameraController.h>
#include <SkyboltSim/CameraController/NullCameraController.h>
#include <SkyboltSim/CameraController/PlanetCameraController.h>
#include <SkyboltSim/CameraController/CameraControllerSelector.h>
#include <SkyboltSim/Components/AssetDescriptionComponent.h>
#include <SkyboltSim/Components/AtmosphereComponent.h>
#include <SkyboltSim/Components/AttacherComponent.h>
#include <SkyboltSim/Components/AttachmentPointsComponent.h>
#include <SkyboltSim/Components/CloudComponent.h>
#include <SkyboltSim/Components/CameraComponent.h>
#include <SkyboltSim/Components/CameraControllerComponent.h>
#include <SkyboltSim/Components/ControlInputsComponent.h>
#include <SkyboltSim/Components/SimpleDynamicBodyComponent.h>
#include <SkyboltSim/Components/SunComponent.h>
#include <SkyboltSim/Components/FuselageComponent.h>
#include <SkyboltSim/Components/JetTurbineComponent.h>
#include <SkyboltSim/Components/MainRotorComponent.h>
#include <SkyboltSim/Components/Motion.h>
#include <SkyboltSim/Components/Node.h>
#include <SkyboltSim/Components/OceanComponent.h>
#include <SkyboltSim/Components/PlanetComponent.h>
#include <SkyboltSim/Components/PropellerComponent.h>
#include <SkyboltSim/Components/ReactionControlSystemComponent.h>
#include <SkyboltSim/Components/RocketMotorComponent.h>
#include <SkyboltSim/Components/ShipWakeComponent.h>
#include <SkyboltSim/Serialization/Serialization.h>
#include <SkyboltVis/ElevationProvider/TilePlanetAltitudeProvider.h>
#include <SkyboltVis/Renderable/Planet/Tile/TileSource/JsonTileSourceFactory.h>

namespace skybolt {

using namespace sim;

sim::ComponentPtr ComponentFactoryFunctionAdapter::create(sim::Entity* entity, const ComponentFactoryContext& context, const nlohmann::json& json)
{
	sim::ComponentPtr component = mFunction(entity, context, json);
	if (component)
	{
		// If component was created, read reflected properties from json
		refl::Instance instance = refl::makeRefInstance(*context.typeRegistry, component.get());
		readReflectedObject(*context.typeRegistry, instance, json);
	}
	return component;
}

static sim::ComponentPtr loadFuselage(Entity* entity, const ComponentFactoryContext& context, const nlohmann::json& json)
{
	FuselageParams params;
	params.liftSlope = readOptionalOrDefault(json, "liftSlope", 5.7);
	params.zeroLiftAlpha = readOptionalOrDefault(json, "zeroLiftAlpha", 0.0);
	params.stallAlpha = readOptionalOrDefault(json, "stallAlpha", 0.3);
	params.stallLift = readOptionalOrDefault(json, "stallLift", 1.0);
	params.liftArea = readOptionalOrDefault(json, "liftArea", 0.0);

	params.dragConstant = readOptionalVector3(json, "dragConstant");

	// Read wing parameters
	params.effectiveWingSpan = readOptionalOrDefault(json, "effectiveWingSpan", 10.0);
	params.wingOswaldEfficiencyFactor = readOptionalOrDefault(json, "wingOswaldEfficiencyFactor", 0.75);

	// Read aerodynamic derivatives
	params.rollAccelDueToSideSlipAngle = readOptionalOrDefault(json, "rollAccelDueToSideSlipAngle", 0.0);
	params.rollAccelDueToRollRate = readOptionalOrDefault(json, "rollAccelDueToRollRate", -1.0);
	params.rollAccelDueToYawRate = readOptionalOrDefault(json, "rollAccelDueToYawRate", 0.0);
	params.rollAccelDueToAileron = readOptionalOrDefault(json, "rollAccelDueToAileron", 0.0);

	params.pitchAccelDueToAngleOfAttack = readOptionalOrDefault(json, "pitchAccelDueToAngleOfAttack", -0.2);
	params.pitchAccelDueToPitchRate = readOptionalOrDefault(json, "pitchAccelDueToPitchRate", -1.0);
	params.pitchAccelDueToElevator = readOptionalOrDefault(json, "pitchAccelDueToElevator", 0.0);

	params.yawAccelDueToSideSlipAngle = readOptionalOrDefault(json, "yawAccelDueToSideSlipAngle", -2.0);
	params.yawAccelDueToRollRate = readOptionalOrDefault(json, "yawAccelDueToRollRate", 0.0);
	params.yawAccelDueToYawRate = readOptionalOrDefault(json, "yawAccelDueToYawRate", -1.0);
	params.yawAccelDueToRudder = readOptionalOrDefault(json, "yawAccelDueToRudder", 0.0);

	params.pitchBaseAccel = readOptionalOrDefault(json, "pitchBaseAccel", 0.0);

	params.aerodynamicDerivativeReferenceSpeed = readOptionalOrDefault(json, "aerodynamicDerivativeReferenceSpeed", 100);

	FuselageComponentConfig config;
	config.params = params;
	config.node = entity->getFirstComponentRequired<Node>().get();
	config.motion = entity->getFirstComponentRequired<Motion>().get();
	config.body = entity->getFirstComponentRequired<DynamicBodyComponent>().get();

	auto inputs = entity->getFirstComponent<ControlInputsComponent>();
	bool hasControlSurfaces = readOptionalOrDefault(json, "hasControlSurfaces", false);
	if (inputs && hasControlSurfaces)
	{
		config.stickInput = inputs->createOrGet("stick", glm::vec2(0), posNegUnitRange<glm::vec2>());
		config.stickTrimInput = inputs->createOrGet("stickTrim", glm::vec2(0), posNegUnitRange<glm::vec2>());
		config.rudderInput = inputs->createOrGet("pedal", 0.0f, posNegUnitRange<float>());

	}
	return std::make_shared<FuselageComponent>(config);
}

static sim::ComponentPtr loadMainRotor(Entity* entity, const ComponentFactoryContext& context, const nlohmann::json& json)
{
	MainRotorParamsPtr params(new MainRotorParams);

	params->maxRpm = json.at("maxRpm").get<double>();

	float surfaceAreaPerBlade = readOptionalOrDefault(json, "surfaceAreaPerBlade", 4.5f);
	int bladeCount = readOptionalOrDefault(json, "bladeCount", 4);

	params->minCollectivePitch = readOptionalOrDefault(json, "minCollectivePitch", 0)  * skybolt::math::degToRadD();
	params->maxCollectivePitch = readOptionalOrDefault(json, "maxCollectivePitch", 12)  * skybolt::math::degToRadD();
	params->maxTppPitch = readOptionalOrDefault(json, "maxTppPitch", 6)  * skybolt::math::degToRadD();
	params->maxTppRoll = readOptionalOrDefault(json, "maxTppRoll", 4)  * skybolt::math::degToRadD();
	params->planAreaOfAllBlades = surfaceAreaPerBlade * bladeCount;
	params->diskRadius = readOptionalOrDefault(json, "diskRadius", 7.5);
	params->zeroLiftAngleOfAttack = readOptionalOrDefault(json, "zeroLiftAngleOfAttack", 0.0);
	params->bladeStallAngleOfAttack = readOptionalOrDefault(json, "bladeStallAngleOfAttack", 15 * skybolt::math::degToRadD());
	params->bladeLiftSlopePerRadian = readOptionalOrDefault(json, "bladeLiftSlopePerRadian", 5.7);
	params->stallLiftCoefficient = readOptionalOrDefault(json, "stallLiftCoefficient", 0.7);
	params->inducedVelocity = math::readOptionalScalarOrCurve(json, "inducedVelocity", 10.0); // Use reasonable default if not specified

	auto inputsComponent = entity->getFirstComponentRequired<ControlInputsComponent>();

	auto component = std::make_shared<MainRotorComponent>(MainRotorComponentConfig{
	.params = params,
	.node = entity->getFirstComponentRequired<Node>().get(),
	.motion = entity->getFirstComponentRequired<Motion>().get(),
	.body = entity->getFirstComponent<DynamicBodyComponent>().get(),
	.positionRelBody = readVector3(json.at("positionRelBody")),
	.orientationRelBody = readOptionalQuaternion(json, "orientationRelBody"),
	.cyclicInput = inputsComponent->createOrGet("stick", glm::vec2(0), posNegUnitRange<glm::vec2>()),
	.cyclicTrimInput = inputsComponent->createOrGet("stickTrim", glm::vec2(0), posNegUnitRange<glm::vec2>()),
	.collectiveInput = inputsComponent->createOrGet("collective", 0.0f, unitRange<float>())
	});
	component->setNormalizedRpm(1.0f);
	return component;
}

static sim::ComponentPtr loadTailRotor(Entity* entity, const ComponentFactoryContext& context, const nlohmann::json& json)
{
	PropellerParams params;
	params.minPitch = readOptionalOrDefault(json, "minPitch", -20)  * skybolt::math::degToRadF();
	params.maxPitch = readOptionalOrDefault(json, "maxPitch", 20)  * skybolt::math::degToRadF();
	params.pitchResponseRate = readOptionalOrDefault(json, "pitchResponseRate", 10);
	params.rpmMultiplier = json.at("rpmMultiplier").get<double>();
 	params.thrustPerRpmPerPitch = readOptionalOrDefault(json, "thrustPerRpmPerPitch", 10.0);

	auto component = std::make_shared< PropellerComponent>(PropellerComponentConfig{
	.params = params,
	.node = entity->getFirstComponentRequired<Node>().get(),
	.body = entity->getFirstComponent<DynamicBodyComponent>().get(),
	.positionRelBody = readVector3(json.at("positionRelBody")),
	.orientationRelBody = readQuaternion(json.at("orientationRelBody")),
	.input = entity->getFirstComponentRequired<ControlInputsComponent>()->createOrGet("pedal", 0.0f, posNegUnitRange<float>()),
	.pitch = 0.0f
	});
	component->setDriverRpm(1.0f);
	return component;
}

sim::ComponentPtr loadReactonControlSystem(Entity* entity, const ComponentFactoryContext& context, const nlohmann::json& json)
{
	ReactionControlSystemParams params;
	params.torque = readVector3(json.at("torque"));

	auto inputsComponent = entity->getFirstComponentRequired<ControlInputsComponent>();

	ReactionControlSystemComponentConfig config;
	config.params = params;
	config.node = entity->getFirstComponentRequired<Node>().get();
	config.body = entity->getFirstComponentRequired<DynamicBodyComponent>().get();
	config.stick = inputsComponent->createOrGet("stick", glm::vec2(0), posNegUnitRange<glm::vec2>());
	config.pedal = inputsComponent->createOrGet("pedal", 0.0f, posNegUnitRange<float>());

	return std::make_shared<ReactionControlSystemComponent>(config);
}

static sim::ComponentPtr loadRocketMotor(Entity* entity, const ComponentFactoryContext& context, const nlohmann::json& json)
{
	RocketMotorComponentParams params;
	params.maxThrust = json.at("maxThrust");

	auto input = entity->getFirstComponentRequired<ControlInputsComponent>()->createOrGet("throttle", 0.0f,unitRange<float>());
	return std::make_shared<RocketMotorComponent>(params, entity->getFirstComponentRequired<Node>().get(), entity->getFirstComponentRequired<DynamicBodyComponent>().get(), input);
}

static sim::ComponentPtr loadShipWake(Entity* entity, const ComponentFactoryContext& context, const nlohmann::json& json)
{
	auto component = std::make_shared<ShipWakeComponent>();
	std::string type = json.at("type");
	if (type == "shipWake")
	{
		float startAheadDistance = readOptionalOrDefault(json, "startAheadDistance", 0); //@ deprecated

		component->type = ShipWakeComponent::Type::SHIP_WAKE;
		component->bowWakeForwardOffset = readOptionalOrDefault(json, "bowWakeForwardOffset", startAheadDistance);
		component->sternWakeForwardOffset = readOptionalOrDefault(json, "sternWakeForwardOffset", 0);
		component->startWidth = readOptionalOrDefault(json, "startWidth", 16.f);
		component->spreadAngularWidth = readOptionalOrDefault(json, "spreadAngularWidth", 5 * math::degToRadD());
		component->length = readOptionalOrDefault(json, "length", 700.f);
	}
	else if (type == "rotorWash")
	{
		component->type = ShipWakeComponent::Type::ROTOR_WASH;
	}
	else
	{
		throw Exception("Unsupported ocean decal type: " + type);
	}
	return component;
}

static sim::ComponentPtr loadAtmosphere(Entity* entity, const ComponentFactoryContext& context, const nlohmann::json& json)
{
	return std::make_shared<AtmosphereComponent>();
}

static sim::ComponentPtr loadNode(Entity* entity, const ComponentFactoryContext& context, const nlohmann::json& json)
{
	return std::make_shared<Node>();
}

static sim::ComponentPtr loadMotion(Entity* entity, const ComponentFactoryContext& context, const nlohmann::json& json)
{
	return std::make_shared<Motion>();
}

static sim::ComponentPtr loadDynamicBody(Entity* entity, const ComponentFactoryContext& context, const nlohmann::json& json)
{
	Node* node = entity->getFirstComponentRequired<Node>().get();
	Motion* motion = entity->getFirstComponentRequired<Motion>().get();
	double mass = json.at("mass");
	Vector3 momentOfInertia = readOptionalVector3(json, "momentOfInertia");

	auto component = std::make_shared<SimpleDynamicBodyComponent>(node, motion, mass, momentOfInertia);
	component->setCenterOfMass(readOptionalVector3(json, "centerOfMass"));
	return component;
}

static sim::ComponentPtr loadAttacher(Entity* entity, const ComponentFactoryContext& context, const nlohmann::json& json)
{
	return std::make_shared<AttacherComponent>(context.simWorld, entity);
}

static sim::ComponentPtr loadCamera(Entity* entity, const ComponentFactoryContext& context, const nlohmann::json& json)
{
	return std::make_shared<CameraComponent>();
}

static sim::ComponentPtr loadAttachmentPoint(Entity* entity, const ComponentFactoryContext& context, const nlohmann::json& json)
{
	auto point = std::make_shared<AttachmentPoint>();
	point->positionRelBody = readVector3(json.at("positionRelBody"));
	point->orientationRelBody = readOptionalQuaternion(json, "orientationRelBody");

	std::string name = json.at("name");
	addAttachmentPoint(*entity, name, point);
	return nullptr; // addAttachmentPoint() will attach the component. TODO: refactor the loadXXX functions to modify the entity and not return anything?
}

static sim::ComponentPtr loadCameraController(Entity* entity, const ComponentFactoryContext& context, const nlohmann::json& json)
{
	auto modifierFactories = context.factoryRegistries->getFirstItemOfType<CameraModifierFactoryRegistry>();
	if (!modifierFactories)
	{
		throw Exception("CameraControllerComponent requires CameraModifierFactoryRegistry to be registered in the factory registries");
	}

	std::map<std::string, CameraControllerPtr> controllers;	
	{
		AttachedCameraController::Params params;
		params.attachmentPointName = "cockpit";
		CameraControllerPtr controller(new AttachedCameraController(entity, context.simWorld, params));
		controllers["Cockpit"] = controller;
	}

	{
		CameraControllerPtr controller(new FreeCameraController(entity, modifierFactories));
		controllers["Free"] = controller;
	}

	{
		OrbitCameraController::Params params(10, 200, 0.5);
		CameraControllerPtr controller(new OrbitCameraController(entity, context.simWorld, params, modifierFactories));
		controllers["Follow"] = controller;
	}

	{
		PlanetCameraController::Params params;
		params.zoomRate = 0.5;
		params.maxDistOnRadius = 7.0;
		CameraControllerPtr controller(new PlanetCameraController(entity, context.simWorld, params));
		controllers["Globe"] = controller;
	}

	{
		CameraControllerPtr controller(new NullCameraController(entity));
		controllers["Null"] = controller;
	}

	auto component = std::make_shared<CameraControllerComponent>(controllers);
	component->selectController("Globe");
	return component;
}

static sim::ComponentPtr loadControlInputs(Entity* entity, const ComponentFactoryContext& context, const nlohmann::json& json)
{
	return std::make_shared<ControlInputsComponent>();
}

static sim::ComponentPtr loadAssetDescription(Entity* entity, const ComponentFactoryContext& context, const nlohmann::json& json)
{
	auto desc = std::make_shared<AssetDescription>();
	desc->description = json.at("description").get<std::string>();

	auto it = json.find("sourceUrl");
	if (it != json.end())
	{
		desc->sourceUrl = it->get<std::string>();
	}

	for (const auto& author : json.at("authors"))
	{
		desc->authors.push_back(author.get<std::string>());
	}

	return std::make_shared<AssetDescriptionComponent>(desc);
}

static sim::ComponentPtr loadScenarioMetadata(Entity* entity, const ComponentFactoryContext& context, const nlohmann::json& json)
{
	auto component = std::make_shared<ScenarioMetadataComponent>();

	ifChildExists(json, "scenarioObjectDirectory", [&](const nlohmann::json& dirJson) {
		if (!dirJson.is_string())
		{
			throw Exception("scenarioObjectDirectory must be a string");
		}
		component->directory = parseStringList(dirJson.get<std::string>(), "/");
	});

	component->userDeletable = readOptionalOrDefault(json, "userDeletable", true);
	component->persistAcrossLoad = readOptionalOrDefault(json, "persistAcrossLoad", false);
	component->replicatable = readOptionalOrDefault(json, "replicatable", true);
	return component;
}

static sim::ComponentPtr loadPlanet(Entity* entity, const ComponentFactoryContext& context, const nlohmann::json& json)
{
	double planetRadius = json.at("radius").get<double>();
	bool hasOcean = readOptionalOrDefault(json, "ocean", true);
	auto planetComponent = std::make_shared<PlanetComponent>(planetRadius);

	if (json.contains("atmosphere"))
	{
		planetComponent->atmosphere = createEarthAtmosphere(); // TODO: use planet specific atmospheric parameters
	}

	// FIXME: This should be split out into a separate component loader, instead of added here as a side effect
	if (hasOcean)
	{
		entity->addComponent(std::make_shared<OceanComponent>());
	}

	return planetComponent;
}

static sim::ComponentPtr loadPlanetElevationTileSource(Entity* entity, const ComponentFactoryContext& context, const nlohmann::json& json)
{
	auto elevationComponent = std::make_shared<PlanetElevationComponent>();

	nlohmann::json elevation = json.at("tileSource");
	elevationComponent->tileSource = context.tileSourceFactoryRegistry->getFactory(elevation.at("format"))(elevation);
	elevationComponent->elevationMaxLodLevel = elevation.at("maxLevel");
	elevationComponent->heightMapTexelsOnTileEdge = readOptionalOrDefault(elevation, "heightMapTexelsOnTileEdge", false);

	auto planetComponent = entity->getFirstComponentRequired<PlanetComponent>();
	planetComponent->altitudeProvider = std::make_shared<vis::NonBlockingTilePlanetAltitudeProvider>(context.scheduler, elevationComponent->tileSource, elevationComponent->elevationMaxLodLevel);

	return elevationComponent;
}

static sim::ComponentPtr loadClouds(Entity* entity, const ComponentFactoryContext& context, const nlohmann::json& json)
{
	return std::make_shared<CloudComponent>();
}

static sim::ComponentPtr loadSun(Entity* entity, const ComponentFactoryContext& context, const nlohmann::json& json)
{
	return std::make_shared<sim::SunComponent>();
}

void addDefaultFactories(ComponentFactoryRegistry& registry)
{
	registry["shipWake"] = std::make_shared<ComponentFactoryFunctionAdapter>(loadShipWake);
	registry["atmosphere"] = std::make_shared<ComponentFactoryFunctionAdapter>(loadAtmosphere);
	registry["attacher"] = std::make_shared<ComponentFactoryFunctionAdapter>(loadAttacher);
	registry["attachmentPoint"] = std::make_shared<ComponentFactoryFunctionAdapter>(loadAttachmentPoint);
	registry["assetDescription"] = std::make_shared<ComponentFactoryFunctionAdapter>(loadAssetDescription);
	registry["camera"] = std::make_shared<ComponentFactoryFunctionAdapter>(loadCamera);
	registry["cameraController"] = std::make_shared<ComponentFactoryFunctionAdapter>(loadCameraController);
	registry["clouds"] = std::make_shared<ComponentFactoryFunctionAdapter>(loadClouds);
	registry["controlInputs"] = std::make_shared<ComponentFactoryFunctionAdapter>(loadControlInputs);
	registry["dynamicBody"] = std::make_shared<ComponentFactoryFunctionAdapter>(loadDynamicBody);
	registry["fuselage"] = std::make_shared<ComponentFactoryFunctionAdapter>(loadFuselage);
	registry["mainRotor"] = std::make_shared<ComponentFactoryFunctionAdapter>(loadMainRotor);
	registry["motion"] = std::make_shared<ComponentFactoryFunctionAdapter>(loadMotion);
	registry["node"] = std::make_shared<ComponentFactoryFunctionAdapter>(loadNode);
	registry["planet"] = std::make_shared<ComponentFactoryFunctionAdapter>(loadPlanet);
	registry["planetElevationTileSource"] = std::make_shared<ComponentFactoryFunctionAdapter>(loadPlanetElevationTileSource);
	registry["reactionControlSystem"] = std::make_shared<ComponentFactoryFunctionAdapter>(loadReactonControlSystem);
	registry["rocketMotor"] = std::make_shared<ComponentFactoryFunctionAdapter>(loadRocketMotor);
	registry["scenarioMetadata"] = std::make_shared<ComponentFactoryFunctionAdapter>(loadScenarioMetadata);
	registry["sun"] = std::make_shared<ComponentFactoryFunctionAdapter>(loadSun);
	registry["tailRotor"] = std::make_shared<ComponentFactoryFunctionAdapter>(loadTailRotor);
}

} // namespace skybolt
