/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#pragma once

#include "SkyboltSim/Entity.h"
#include "SkyboltSim/SkyboltSimFwd.h"

namespace skybolt {
namespace sim {

class CameraController
{
public:
	struct Input
	{
		static Input zero()
		{
			return {0,0,0,0,0,false,false};
		}

		float forwardSpeed; //!< Typical range is [-1, 1], but can exceed range if needed
		float rightSpeed; //!< Typical range is [-1, 1], but can exceed range if needed
		float yawRate; //!< Radians per second
		float tiltRate; //!< Radians per second
		float zoomRate; //!< Typical range is Range [-1, 1], but can exceed range if needed
		bool modifier1Pressed;
		bool modifier2Pressed;
	};

	CameraController(Entity* camera);
	virtual ~CameraController();

	virtual void updatePostDynamicsSubstep(SecondsD dtSubstep) {}
	virtual void update(SecondsD dt) {}
	virtual void setInput(const Input& input) {}

	virtual void setActive(bool active) { mActive = active; }

protected:
	Node* mNodeComponent;
	CameraComponent* mCameraComponent;
	bool mActive = false;
};

} // namespace sim
} // namespace skybolt