/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#pragma once

#include "SkyboltSim/Component.h"
#include "SkyboltSim/Components/ControlInputsComponent.h"
#include <vector>

class btDynamicsWorld;
class btRaycastVehicle;
class btRigidBody;

namespace skybolt {
namespace sim {

class CustomVehicleRacaster;

struct WheelConfig
{
	Vector3 attachmentPoint;
	double wheelRadius;
	double maxSuspensionTravelLength;
	double stiffness;
	double dampingCompression;
	double dampingRelaxation;
	double maxSteeringAngleRadians = 0;
	bool drivenByEngine = false;
};

struct BulletWheelsComponentConfig
{
	btDynamicsWorld* world;
	btRigidBody* body;
	Real mass;
	std::vector<WheelConfig> wheels;
	ControlInputFloatPtr steering; //!< Optional. If null, steering is disabled.
};

class BulletWheelsComponent : public Component
{
public:
	BulletWheelsComponent(const BulletWheelsComponentConfig& config);
	~BulletWheelsComponent() override;

	void updatePreDynamics(TimeReal dt, TimeReal dtWallClock) override;

	void setDrivingForce(double force);

private:
	btDynamicsWorld* mWorld;
	ControlInputFloatPtr mSteering;
	std::vector<double> mMaxSteeringAnglesRadians;
	std::vector<int> mDrivingWheels;
	std::unique_ptr<btRaycastVehicle> mVehicle;
	std::unique_ptr<CustomVehicleRacaster> mVehicleRayCaster;
};

} // namespace sim
} // namespace skybolt