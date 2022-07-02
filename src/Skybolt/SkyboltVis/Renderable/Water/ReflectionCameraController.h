/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltVis/Camera.h"
#include "SkyboltVis/Scene.h"
#include "SkyboltVis/RenderOperation/RenderTexture.h"

#include <osg/ClipPlane>
#include <osg/Texture2D>

namespace skybolt {
namespace vis {

struct ReflectionCameraControllerConfig
{
	ScenePtr reflectionScene;
	TextureFactory reflectionTextureFactory;
};

class ReflectionCameraController
{
public:
	ReflectionCameraController(const ReflectionCameraControllerConfig& config);
	~ReflectionCameraController();

	void setPlaneZ(float z) { planeZ = z; }

	void update(const Camera& referenceCamera);

	const CameraPtr& getReflectionCamera() const { return mReflectionCamera; }

private:
	CameraPtr mReflectionCamera;
	ScenePtr mReflectionScene;
	osg::ClipPlane* mClipPlane;
	float planeZ;

	osg::Matrixf mReflectionViewProjectionMatrix;
	osg::ref_ptr<RenderTexture> mTarget;
};

} // namespace vis
} // namespace skybolt
