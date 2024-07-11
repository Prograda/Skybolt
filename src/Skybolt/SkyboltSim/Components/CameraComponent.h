/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltSim/Component.h"
#include <SkyboltReflection/Reflection.h>

namespace skybolt {
namespace sim {

struct CameraState
{
	CameraState() :
		fovY(0.5f), nearClipDistance(0.5), farClipDistance(5e7) {}
	double nearClipDistance;
	double farClipDistance;
	float fovY;
};

class CameraComponent : public Component
{
public:
	const CameraState &getState() const {return mState;}
	CameraState &getState() { return mState; }

	// Conveniance methods
	float getFovY() const { return mState.fovY; }
	void setFovY(float v) { mState.fovY = v; }

private:
	CameraState mState;
};

SKYBOLT_REFLECT_EXTERN(CameraComponent)

} // namespace sim
} // namespace skybolt