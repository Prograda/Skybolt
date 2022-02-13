/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include <SkyboltEngine/ComponentFactory.h>
#include <SkyboltEngine/Plugin/Plugin.h>
#include <SkyboltSim/Component.h>
#include <SkyboltSim/Components/ControlInputsComponent.h>
#include <SkyboltSim/Entity.h>
#include <SkyboltSim/Spatial/Orientation.h>
#include <SkyboltSim/Spatial/Position.h>
#include <SkyboltCommon/Math/MathUtility.h>

#include <JSBSim/FGFDMExec.h>
#include <JSBSim/models/FGPropulsion.h>

#include <boost/config.hpp>
#include <boost/dll/alias.hpp>
#include <assert.h>

namespace skybolt {
using namespace sim;

class JsbSimAircraftComponent : public Component
{
public:
	JsbSimAircraftComponent(Entity* entity, std::unique_ptr<JSBSim::FGFDMExec> exec) :
		mEntity(entity),
		mExec(std::move(exec))
	{
		mExec->RunIC();
		mExec->SetPropertyValue("propulsion/set-running", -1); // -1 turns on all engines

		auto controls = entity->getFirstComponentRequired<ControlInputsComponent>();
		mStickInput = controls->createOrGet<glm::vec2>("stick", glm::vec2(0.0f), posNegUnitRange<glm::vec2>());
		mThrottleInput = controls->createOrGet<float>("throttle", 0.0f);
		mRudderInput = controls->createOrGet<float>("rudder", 0.0f, posNegUnitRange<float>());
	}

	void updatePreDynamics(TimeReal dt, TimeReal dtWallClock) override
	{
		auto position = getPosition(*mEntity);
		if (!position)
			return;

		// Set entity position if it was changed externally to JSBSim
		if (!mLastPosition || mLastPosition != *position)
		{
			sim::LatLonAlt lla = sim::toLatLonAlt(GeocentricPosition(*position)).position;
			mExec->SetPropertyValue("position/lat-gc-rad", lla.lat);
			mExec->SetPropertyValue("position/long-gc-rad", lla.lon);
			mExec->SetPropertyValue("position/h-sl-meters", lla.alt);
		}

		// Set flight controls
		mExec->SetPropertyValue("fcs/aileron-cmd-norm", mStickInput->value.x);
		mExec->SetPropertyValue("fcs/elevator-cmd-norm", -mStickInput->value.y);
		mExec->SetPropertyValue("fcs/rudder-cmd-norm", -mRudderInput->value);
		mExec->SetPropertyValue("fcs/throttle-cmd-norm", mThrottleInput->value);

		// Tick JSBSim
		mExec->Run();

		// Get results
		sim::LatLonAlt lla(
			mExec->GetPropertyValue("position/lat-gc-rad"),
			mExec->GetPropertyValue("position/long-gc-rad"),
			mExec->GetPropertyValue("position/h-sl-meters")
		);

		Vector3 newPosition = sim::toGeocentric(LatLonAltPosition(lla)).position;
		setPosition(*mEntity, newPosition);
		mLastPosition = newPosition;

		sim::Vector3 eulerRpy(
			mExec->GetPropertyValue("attitude/roll-rad"),
			mExec->GetPropertyValue("attitude/pitch-rad"),
			mExec->GetPropertyValue("attitude/heading-true-rad") // NOTE: heading is actually yaw
		);

		Quaternion orientation = math::quatFromEuler(eulerRpy);

		setOrientation(*mEntity, toGeocentric(LtpNedOrientation(orientation), toLatLon(lla)).orientation);
	}

private:
	sim::Entity* mEntity;
	std::unique_ptr<JSBSim::FGFDMExec> mExec;
	std::optional<sim::Vector3> mLastPosition;
	std::shared_ptr<ControlInputT<glm::vec2>> mStickInput;
	std::shared_ptr<ControlInputT<float>> mThrottleInput;
	std::shared_ptr<ControlInputT<float>> mRudderInput;
};

static sim::ComponentPtr loadJsbSimAircraftComponent(Entity* entity, const ComponentFactoryContext& context, const nlohmann::json& json)
{
	auto exec = std::make_unique<JSBSim::FGFDMExec>();
	exec->SetRootDir(SGPath("D:/Programming/libs/jsbsim"));
	exec->SetAircraftPath(SGPath("D:/Programming/libs/jsbsim/aircraft"));
	exec->SetEnginePath(SGPath("D:/Programming/libs/jsbsim/engine"));
	exec->SetSystemsPath(SGPath("D:/Programming/libs/jsbsim/systems"));
	exec->LoadModel(json.at("model"));

	assert(entity);
	return std::make_shared<JsbSimAircraftComponent>(entity, std::move(exec));
}

const std::string JsbSimComponentName = "jsbSimAircraft";

class JsbSimPlugin : public Plugin
{
public:
	JsbSimPlugin(const PluginConfig& config) :
		mComponentFactoryRegistry(config.simComponentFactoryRegistry)
	{
		(*mComponentFactoryRegistry)[JsbSimComponentName] = std::make_shared<ComponentFactoryFunctionAdapter>(loadJsbSimAircraftComponent);
	}

	~JsbSimPlugin()
	{
		mComponentFactoryRegistry->erase(JsbSimComponentName);
	}

private:
	ComponentFactoryRegistryPtr mComponentFactoryRegistry;
};

namespace plugins {

	std::shared_ptr<Plugin> createEnginePlugin(const PluginConfig& config)
	{
		return std::make_shared<JsbSimPlugin>(config);
	}

	BOOST_DLL_ALIAS(
		plugins::createEnginePlugin,
		createEnginePlugin
	)
}

} // namespace skybolt {