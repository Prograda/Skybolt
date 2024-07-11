/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "ReflectionCameraController.h"
#include "SkyboltVis/Camera.h"
#include "SkyboltVis/RenderOperation/RenderTexture.h"

using namespace skybolt::vis;

static RenderTextureConfig createRenderTextureConfig(const TextureFactory& factory)
{
	RenderTextureConfig config;
	config.colorTextureFactories = { factory };
	return config;
}

ReflectionCameraController::ReflectionCameraController(const ReflectionCameraControllerConfig& config) :
	mReflectionCamera(new Camera(1.0)),
	mReflectionScene(config.reflectionScene),
	mClipPlane(new osg::ClipPlane),
	planeZ(0),
	mTarget(new RenderTexture(createRenderTextureConfig(config.reflectionTextureFactory)))
{
	mTarget->setCamera(mReflectionCamera);
	mTarget->setScene(mReflectionScene->getBucketGroup(Scene::Bucket::Default));

	mClipPlane->setClipPlaneNum(0);

	// TODO: reenable once gl_ClipDistance is set by all shaders.
	// If it's not set by shader, it's initalized to the wrong value and causes incorrect clipping.
	// mReflectionScene->addClipPlane(mClipPlane);
}

ReflectionCameraController::~ReflectionCameraController()
{
	// TODO: reenable mReflectionScene->removeClipPlane(mClipPlane);
}

void ReflectionCameraController::update(const Camera& referenceCamera)
{
	osg::Vec3 reflectedPos = referenceCamera.getPosition();
	reflectedPos.z() = planeZ - reflectedPos.z();
	mReflectionCamera->setPosition(reflectedPos);

	// mirror orientation about z plane
	osg::Matrix orientationMatrix;
	referenceCamera.getOrientation().get(orientationMatrix);
	orientationMatrix(2, 0) = -orientationMatrix(2, 0);
	orientationMatrix(2, 1) = -orientationMatrix(2, 1);
	orientationMatrix(0, 2) = -orientationMatrix(0, 2);
	orientationMatrix(1, 2) = -orientationMatrix(1, 2);

	mReflectionCamera->setOrientation(orientationMatrix.getRotate());
	mReflectionCamera->setFovY(referenceCamera.getFovY());
	mReflectionCamera->setAspectRatio(referenceCamera.getAspectRatio());

	// update clipping plane to clip above/below the water
	bool isUnderwater = referenceCamera.getPosition().z() > planeZ;

	osg::Plane plane;
	if (isUnderwater)
		plane = osg::Plane(osg::Vec3f(0, 0, 1), -planeZ);
	else
		plane = osg::Plane(osg::Vec3f(0, 0, -1), planeZ);

	mClipPlane->setClipPlane(plane);
}