/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "CameraController.h"
#include "CameraModifierStack.h"
#include "Pitchable.h"
#include "Yawable.h"
#include "SkyboltSim/Serialization/Serialization.h"

namespace skybolt {
namespace sim {

class FreeCameraController : public CameraController, public CameraModifierStack, public ExplicitSerialization, public Pitchable, public Yawable
{
public:
	FreeCameraController(Entity* camera, const CameraModifierFactoryRegistryPtr& cameraModifierFactories);

	void updateTimeStep(const UpdateTimeStepArgs& args) override;
	void setInput(const Input& input) override { mInput = input; }

	void setActive(bool active) override;

	double getZoom() const;
	void setZoom(double zoom);

	double minFovY = math::degToRadD() * 10.0;
	double maxFovY = math::degToRadD() * 120.0;

public: // ExplicitSerialization interface
	nlohmann::json toJson(refl::TypeRegistry& typeRegistry) const;
	void fromJson(refl::TypeRegistry& typeRegistry, const nlohmann::json& j);

private:
	Input mInput = Input::zero();

	Vector3 mBasePosition = {}; //!< The camera position without camera modifiers applied.
	std::optional<Vector3> mPreviousNodePosition;
};

SKYBOLT_REFLECT_EXTERN(FreeCameraController);

} // namespace sim
} // namespace skybolt