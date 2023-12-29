/* Copyright 2012-2020 Matthew Reid
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
#include <SkyboltSim/Components/AttachmentComponent.h>
#include <SkyboltSim/Components/AttachmentPointsComponent.h>
#include <SkyboltSim/Components/CameraComponent.h>
#include <SkyboltSim/Components/CameraControllerComponent.h>
#include <SkyboltSim/Components/ControlInputsComponent.h>
#include <SkyboltSim/Components/SimpleDynamicBodyComponent.h>
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
#include <SkyboltVis/ElevationProvider/TilePlanetAltitudeProvider.h>
#include <SkyboltVis/Renderable/Planet/Tile/TileSource/JsonTileSourceFactory.h>

namespace skybolt {

using namespace sim;

static sim::ComponentPtr loadFuselage(Entity* entity, const ComponentFactoryContext& context, const nlohmann::json& json)
{
	FuselageParams params;
	params.liftSlope = readOptionalOrDefault(json, "liftSlope", 5.7);
	params.zeroLiftAlpha = readOptionalOrDefault(json, "zeroLiftAlpha", 0.0);
	params.stallAlpha = readOptionalOrDefault(json, "stallAlpha", 0.3);
	params.stallLift = readOptionalOrDefault(json, "stallLift", 1.0);
	params.liftArea = readOptionalOrDefault(json, "liftArea", 0.0);

	params.momentMultiplier = json.at("momentMultiplier");

	params.dragConst = readOptionalVector3(json, "dragConstant");

	params.rollDueToSideSlipAngle = readOptionalOrDefault(json, "rollDueToSideSlipAngle", 0.0);
	params.rollDueToRollRate = readOptionalOrDefault(json, "rollDueToRollRate", -2.0);
	params.rollDueToYawRate = readOptionalOrDefault(json, "rollDueToYawRate", 0.0);
	params.rollDueToAileron = readOptionalOrDefault(json, "rollDueToAileron", 0.0);

	params.pitchNeutralMoment = 0;
	params.pitchDueToAngleOfAttack = readOptionalOrDefault(json, "pitchDueToAngleOfAttack", 0.0);
	params.pitchDueToPitchRate = readOptionalOrDefault(json, "pitchDueToPitchRate", -10.0);
	params.pitchDueToElevator = readOptionalOrDefault(json, "pitchDueToElevator", 0.0);

	params.yawDueToSideSlipAngle = readOptionalOrDefault(json, "yawDueToSideSlipAngle", 0.0);
	params.yawDueToRollRate = readOptionalOrDefault(json, "yawDueToRollRate", 0.0);
	params.yawDueToYawRate = readOptionalOrDefault(json, "yawDueToYawRate", -10.0);
	params.yawDueToRudder = readOptionalOrDefault(json, "yawDueToRudder", 0.0);

	params.maxAutoTrimAngleOfAttack = readOptionalOrDefault(json, "maxAutoTrimAngleOfAttack", 0.5);

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
		config.rudderInput = inputs->createOrGet("rudder", 0.0f, posNegUnitRange<float>());

	}
	return std::make_shared<FuselageComponent>(config);
}

static sim::ComponentPtr loadMainRotor(Entity* entity, const ComponentFactoryContext& context, const nlohmann::json& json)
{
	MainRotorParamsPtr params(new MainRotorParams);

	params->maxRpm = json.at("maxRpm").get<double>();

	float surfaceAreaPerBlade = readOptionalOrDefault(json, "surfaceAreaPerBlade", 1.3f);
	int bladeCount = readOptionalOrDefault(json, "bladeCount", 4);

	// TODO: read from json
	params->pitchResponseRate = 3;
	params->minPitch = 0.02;
	params->pitchRange = 0.12;
	params->maxTppPitch = 0.1;
	params->maxTppRoll = 0.05;
	params->tppPitchOffset = readOptionalOrDefault(json, "tppPitchOffset", -3.f)  * skybolt::math::degToRadF();
	params->liftConst = 0.5f * 5.9f * surfaceAreaPerBlade * bladeCount; // 0.5 * liftSlope[1/rad] * bladeSurfaceArea * bladeCount
	params->diskRadius = readOptionalOrDefault(json, "diskRadius", 7.3f);
	params->zeroLiftAlpha = 0;

	auto inputsComponent = entity->getFirstComponentRequired<ControlInputsComponent>();

	MainRotorComponentConfig config;
	config.params = params;
	config.node = entity->getFirstComponentRequired<Node>().get();
	config.motion = entity->getFirstComponentRequired<Motion>().get();
	config.body = entity->getFirstComponentRequired<DynamicBodyComponent>().get();
	config.positionRelBody = readVector3(json.at("positionRelBody"));
	config.orientationRelBody = readQuaternion(json.at("orientationRelBody"));
	config.cyclicInput = inputsComponent->createOrGet("stick", glm::vec2(0), posNegUnitRange<glm::vec2>());
	config.collectiveInput = inputsComponent->createOrGet("collective", 0.0f, unitRange<float>());

	Vector3 positionRelBody = readVector3(json.at("positionRelBody"));
	Quaternion orientationRelBody = readOptionalQuaternion(json, "orientationRelBody");

	auto component = std::make_shared<MainRotorComponent>(config);
	component->setNormalizedRpm(1.0f);
	return component;
}

static sim::ComponentPtr loadTailRotor(Entity* entity, const ComponentFactoryContext& context, const nlohmann::json& json)
{
	PropellerParams params;
	params.minPitch = -0.1;
	params.pitchRange = 0.2;
	params.pitchResponseRate = 10;
	params.rpmMultiplier = json.at("rpmMultiplier").get<double>();
 	params.thrustPerRpmPerPitch = 10;

	PropellerComponentConfig config;
	config.params = params;
	config.node = entity->getFirstComponentRequired<Node>().get();
	config.body = entity->getFirstComponentRequired<DynamicBodyComponent>().get();
	config.positionRelBody = readVector3(json.at("positionRelBody"));
	config.orientationRelBody = readQuaternion(json.at("orientationRelBody"));
	config.input = entity->getFirstComponentRequired<ControlInputsComponent>()->createOrGet("pedal", 0.0f, posNegUnitRange<float>());
	config.pitch = 0.0f;

	auto component = std::make_shared< PropellerComponent>(config);
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
		component->type = ShipWakeComponent::Type::SHIP_WAKE;
		component->startRadius = readOptionalOrDefault(json, "startRadius", 8.f);
		component->endRadius = readOptionalOrDefault(json, "endRadius", 40.f);
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

static sim::ComponentPtr loadAttachment(Entity* entity, const ComponentFactoryContext& context, const nlohmann::json& json)
{
	sim::AttachmentParams params;
	params.positionRelBody = readVector3(json.at("positionRelBody"));
	params.orientationRelBody = readOptionalQuaternion(json, "orientationRelBody");

	return std::make_shared<AttachmentComponent>(params, context.simWorld, entity);
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
	std::map<std::string, CameraControllerPtr> controllers;	
	{
		AttachedCameraController::Params params;
		params.minFovY = 0.3;
		params.maxFovY = 1.3;
		params.attachmentPointName = "cockpit";
		CameraControllerPtr controller(new AttachedCameraController(entity, context.simWorld, params));
		controllers["Cockpit"] = controller;
	}

	{
		FreeCameraController::Params params;
		params.fovY = 0.5;
		CameraControllerPtr controller(new FreeCameraController(entity, params));
		controllers["Free"] = controller;
	}

	{
		OrbitCameraController::Params params(10, 200, 0.5);
		CameraControllerPtr controller(new OrbitCameraController(entity, context.simWorld, params));
		controllers["Follow"] = controller;
	}

	{
		PlanetCameraController::Params params;
		params.zoomRate = 0.5;
		params.maxDistOnRadius = 7.0;
		params.fovY = 0.5f;
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
	component->directory = parseStringList(json.at("scenarioObjectDirectory").get<std::string>(), "/");
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

void addDefaultFactories(ComponentFactoryRegistry& registry)
{
	registry["shipWake"] = std::make_shared<ComponentFactoryFunctionAdapter>(loadShipWake);
	registry["attachment"] = std::make_shared<ComponentFactoryFunctionAdapter>(loadAttachment);
	registry["attachmentPoint"] = std::make_shared<ComponentFactoryFunctionAdapter>(loadAttachmentPoint);
	registry["assetDescription"] = std::make_shared<ComponentFactoryFunctionAdapter>(loadAssetDescription);
	registry["camera"] = std::make_shared<ComponentFactoryFunctionAdapter>(loadCamera);
	registry["cameraController"] = std::make_shared<ComponentFactoryFunctionAdapter>(loadCameraController);
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
	registry["tailRotor"] = std::make_shared<ComponentFactoryFunctionAdapter>(loadTailRotor);
}

} // namespace skybolt
