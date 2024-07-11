/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#pragma once

#include "SimVisBinding.h"

namespace skybolt {

class CameraSimVisBinding : public SimVisBinding
{
public:
	CameraSimVisBinding(const sim::Entity* simCamera, const vis::CameraPtr& camera);

	// sync vis object to the sim object
	void syncVis(const GeocentricToNedConverter& converter) override;

	const vis::CameraPtr& getCamera() const { return mCamera; }

private:
	const sim::Entity* mEntity;
	vis::CameraPtr mCamera;
	const sim::CameraComponent* mCameraComponent;
};

//! @return a camera attached to the entity, or nullptr if the entity has no camera
vis::CameraPtr getVisCamera(const sim::Entity& entity);

} // namespace skybolt