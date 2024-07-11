/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#include "CameraSimVisBinding.h"
#include "GeocentricToNedConverter.h"
#include <SkyboltVis/Camera.h>
#include <SkyboltSim/Entity.h>
#include <SkyboltSim/Components/CameraComponent.h>

namespace skybolt {

CameraSimVisBinding::CameraSimVisBinding(const sim::Entity* simCamera, const vis::CameraPtr& visCamera) :
	mEntity(simCamera),
	mCamera(visCamera),
	mCameraComponent(simCamera->getFirstComponent<sim::CameraComponent>().get())
{
	assert(mEntity);
	assert(mCamera);
	assert(mCameraComponent);
}

void CameraSimVisBinding::syncVis(const GeocentricToNedConverter& converter)
{
	osg::Quat q = converter.convert(*sim::getOrientation(*mEntity));
	mCamera->setOrientation(q);

	osg::Vec3d p = converter.convertPosition(*sim::getPosition(*mEntity));
	mCamera->setPosition(p);
	
	const sim::CameraState& state = mCameraComponent->getState();
	
	mCamera->setFovY(state.fovY);
	mCamera->setNearClipDistance(state.nearClipDistance);
	mCamera->setFarClipDistance(state.farClipDistance);
}

vis::CameraPtr getVisCamera(const sim::Entity& camera)
{
	return static_cast<const CameraSimVisBinding&>(*camera.getFirstComponent<SimVisBindingsComponent>()->bindings.front()).getCamera();
}

} // namespace skybolt