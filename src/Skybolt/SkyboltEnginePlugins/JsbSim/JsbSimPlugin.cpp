#include <SkyboltEngine/ComponentFactory.h>
#include <SkyboltEngine/EngineRoot.h>
#include <SkyboltEngine/Plugin/Plugin.h>
#include <SkyboltSim/Component.h>
#include <SkyboltSim/Components/ControlInputsComponent.h>
#include <SkyboltSim/Components/Motion.h>
#include <SkyboltSim/Entity.h>
#include <SkyboltSim/Spatial/Geocentric.h>
#include <SkyboltSim/Spatial/Orientation.h>
#include <SkyboltSim/Spatial/Position.h>
#include <SkyboltCommon/Json/JsonHelpers.h>
#include <SkyboltCommon/Math/MathUtility.h>
#include <SkyboltCommon/TypeMatcher.h>

#include <JSBSim/FGFDMExec.h>
#include <JSBSim/initialization/FGTrim.h>
#include <JSBSim/models/FGPropulsion.h>

#include <boost/config.hpp>
#include <boost/dll/alias.hpp>
#include <assert.h>

namespace skybolt {
using namespace sim;

constexpr double feetToMeters = 0.3048;
constexpr double metersToFeet = 1.0 / feetToMeters;

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
		mRudderInput = controls->createOrGet<float>("pedal", 0.0f, posNegUnitRange<float>());
	}

	SKYBOLT_BEGIN_REGISTER_UPDATE_HANDLERS
		SKYBOLT_REGISTER_UPDATE_HANDLER(sim::UpdateStage::PreDynamicsSubStep, updatePreDynamics)
	SKYBOLT_END_REGISTER_UPDATE_HANDLERS

	void updatePreDynamics()
	{
		auto currentPosition = getPosition(*mEntity);
		if (!currentPosition)
			return;

		auto currentOrientation = getOrientation(*mEntity);
		if (!currentOrientation)
			return;

		Vector3 currentVelocity = getVelocity(*mEntity).value_or(math::dvec3Zero());

		// Set entity position if it was changed externally to JSBSim
		if (!mLastPosition || mLastPosition != *currentPosition)
		{
			sim::LatLonAlt lla = sim::toLatLonAlt(GeocentricPosition(*currentPosition)).position;
			if (!mInitialConditionsSet)
			{
				JSBSim::FGInitialCondition* IC = mExec->GetIC();
				IC->SetLatitudeRadIC(lla.lat);
				IC->SetLongitudeRadIC(lla.lon);
				IC->SetAltitudeASLFtIC(lla.alt * metersToFeet);
			}
			else
			{
				JSBSim::FGPropagate* propagate = mExec->GetPropagate();
				propagate->SetPosition(lla.lat, lla.lon, lla.alt * metersToFeet);
			}
		}

		// Set entity orientation if it was changed externally to JSBSim
		if (!mLastOrientation || mLastOrientation != currentOrientation)
		{
			Quaternion ltpNed = toLtpNed(GeocentricOrientation(*currentOrientation), geocentricToLatLon(*currentPosition)).orientation;
			Vector3 rpy = math::eulerFromQuat(ltpNed);

			if (!mInitialConditionsSet)
			{
				JSBSim::FGInitialCondition* IC = mExec->GetIC();
				IC->SetPhiRadIC(rpy.x);
				IC->SetThetaRadIC(rpy.y);
				IC->SetPsiRadIC(rpy.z);
			}
			else
			{
				JSBSim::FGPropagate* propagate = mExec->GetPropagate();
				JSBSim::FGQuaternion new_orient(rpy.x, rpy.y, rpy.z);
				propagate->SetInertialOrientation(new_orient);

				mExec->SetPropertyValue("attitude/roll-rad", rpy.x);
				mExec->SetPropertyValue("attitude/pitch-rad", rpy.y);
				mExec->SetPropertyValue("attitude/heading-true-rad", rpy.z); // NOTE: heading is yaw
			}
		}

		// Set entity velocity if it was changed externally to JSBSim
		if (!mLastVelocity || mLastVelocity != currentVelocity)
		{
			Matrix3 ltpOrientation = geocentricToLtpOrientation(*currentPosition);
			Vector3 currentVelocityNed = glm::inverse(ltpOrientation) * currentVelocity;

			if (!mInitialConditionsSet)
			{
				JSBSim::FGInitialCondition* IC = mExec->GetIC();
				IC->SetVNorthFpsIC(currentVelocityNed.x * metersToFeet);
				IC->SetVEastFpsIC(currentVelocityNed.y * metersToFeet);
				IC->SetVDownFpsIC(currentVelocityNed.z * metersToFeet);
			}
			else
			{
				JSBSim::FGPropagate* propagate = mExec->GetPropagate();
				propagate->SetInertialVelocity(JSBSim::FGColumnVector3(
					currentVelocityNed.x * metersToFeet,
					currentVelocityNed.y * metersToFeet,
					currentVelocityNed.z * metersToFeet
				));
			}
		}

		// Set flight controls
		mExec->SetPropertyValue("fcs/aileron-cmd-norm", mStickInput->value.x);
		mExec->SetPropertyValue("fcs/elevator-cmd-norm", -mStickInput->value.y);
		mExec->SetPropertyValue("fcs/rudder-cmd-norm", -mRudderInput->value);
		mExec->SetPropertyValue("fcs/throttle-cmd-norm", mThrottleInput->value);

		// Load initial conditions before first tick
		if (!mInitialConditionsSet)
		{
			mExec->RunIC();
			mInitialConditionsSet = true;
		}

		// Tick JSBSim
		mExec->Run();

		// Get results
		// Update position
		sim::LatLonAlt lla(
			mExec->GetPropertyValue("position/lat-gc-rad"),
			mExec->GetPropertyValue("position/long-gc-rad"),
			mExec->GetPropertyValue("position/h-sl-meters")
		);

		Vector3 newPosition = sim::toGeocentric(LatLonAltPosition(lla)).position;
		setPosition(*mEntity, newPosition);
		mLastPosition = newPosition;

		// Update orientation
		sim::Vector3 eulerRpy(
			mExec->GetPropertyValue("attitude/roll-rad"),
			mExec->GetPropertyValue("attitude/pitch-rad"),
			mExec->GetPropertyValue("attitude/heading-true-rad") // NOTE: heading is yaw
		);

		Quaternion ltpNedOrientation = math::quatFromEuler(eulerRpy);
		Quaternion orientation = toGeocentric(LtpNedOrientation(ltpNedOrientation), toLatLon(lla)).orientation;
		setOrientation(*mEntity, orientation);
		mLastOrientation = orientation;

		// Update linear velocity
		Matrix3 ltpOrientation = geocentricToLtpOrientation(*currentPosition);
		sim::Vector3 velocity = ltpOrientation * sim::Vector3(
			mExec->GetPropertyValue("velocities/v-north-fps"),
			mExec->GetPropertyValue("velocities/v-east-fps"),
			mExec->GetPropertyValue("velocities/v-down-fps")
		) * feetToMeters;
		setVelocity(*mEntity, velocity);
		mLastVelocity = velocity;

		//update angular velocity in body axes
		if (auto motion = mEntity->getFirstComponent<Motion>(); motion)
		{
			motion->angularVelocity = orientation * sim::Vector3(
				mExec->GetPropertyValue("velocities/p-rad_sec"),
				mExec->GetPropertyValue("velocities/q-rad_sec"),
				mExec->GetPropertyValue("velocities/r-rad_sec"));
		}
	}

	void setSimTime(SecondsD time) override
	{
		if (time == 0)
		{
			// Reset initial conditions
			mInitialConditionsSet = false;
		}
	}

	void advanceSimTime(SecondsD newTime, SecondsD dt) override
	{
		mExec->Setdt(dt);
	}

private:
	sim::Entity* mEntity;
	std::unique_ptr<JSBSim::FGFDMExec> mExec;

	std::optional<sim::Vector3> mLastPosition;
	std::optional<sim::Quaternion> mLastOrientation;
	std::optional<sim::Vector3> mLastVelocity;
	bool mInitialConditionsSet = false;

	std::shared_ptr<ControlInputT<glm::vec2>> mStickInput;
	std::shared_ptr<ControlInputT<float>> mThrottleInput;
	std::shared_ptr<ControlInputT<float>> mRudderInput;
};

static sim::ComponentPtr loadJsbSimAircraftComponent(Entity* entity, const ComponentFactoryContext& context, const nlohmann::json& json, const std::string& defaultJsbSimRootDir)
{
	std::string jsbSimRoot = readOptionalOrDefault(json, "jsbSimRootDir", defaultJsbSimRootDir);

	auto exec = std::make_unique<JSBSim::FGFDMExec>();
	exec->SetDebugLevel(0); // disable JSBSim debug output
	exec->SetRootDir(SGPath(jsbSimRoot));
	exec->SetAircraftPath(SGPath(jsbSimRoot + "/aircraft"));
	exec->SetEnginePath(SGPath(jsbSimRoot + "/engine"));
	exec->SetSystemsPath(SGPath(jsbSimRoot + "/systems"));
	exec->LoadModel(json.at("model"));

	assert(entity);
	return std::make_shared<JsbSimAircraftComponent>(entity, std::move(exec));
}

static std::string readDefaultJsbSimRootDir(const nlohmann::json& j)
{
	std::string rootDir;
	ifChildExists(j, "jsb", [&] (const nlohmann::json& child) {
		rootDir = child.at("rootDir");
		});

	if (rootDir.empty())
	{
		rootDir = ".";
		BOOST_LOG_TRIVIAL(warning) << "'jsb.rootDir' setting not found in engine settings file. Setting to default of '" << rootDir << "'";
	}

	return rootDir;
}

const std::string JsbSimComponentName = "jsbSimAircraft";

class JsbSimPlugin : public Plugin
{
public:
	JsbSimPlugin(const PluginConfig& config) :
		mComponentFactoryRegistry(valueOrThrowException(getExpectedRegistry<ComponentFactoryRegistry>(*config.engineRoot->factoryRegistries)))
	{
		const nlohmann::json& settings = config.engineRoot->engineSettings;
		std::string defaultJsbSimRootDir = readDefaultJsbSimRootDir(settings);

		(*mComponentFactoryRegistry)[JsbSimComponentName] = std::make_shared<ComponentFactoryFunctionAdapter>([defaultJsbSimRootDir] (Entity* entity, const ComponentFactoryContext& context, const nlohmann::json& json) {
			return loadJsbSimAircraftComponent(entity, context, json, defaultJsbSimRootDir);
			});
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