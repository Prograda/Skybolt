/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "ViewportPropertiesModel.h"
#include <SkyboltCommon/Math/MathUtility.h>
#include <SkyboltSim/Components/CameraComponent.h>
#include <SkyboltVis/Scene.h>

using namespace skybolt;

ViewportPropertiesModel::ViewportPropertiesModel(vis::Scene* Scene, sim::CameraComponent* camera) :
	mScene(Scene)
{
	{
		mFov = createQtProperty("Vertical FOV", 0.0);
		mFov->setValue(camera->getState().fovY * skybolt::math::radToDegF());
		mProperties.push_back(mFov);

		connect(mFov.get(), &QtProperty::valueChanged, [=]()
		{
			camera->getState().fovY = mFov->value.toFloat() * skybolt::math::degToRadF();
		});
	}
	{
		mAmbientLight = createQtProperty("Ambient Light", 0.0);
		mAmbientLight->setValue(mScene->getAmbientLightColor().x());
		mProperties.push_back(mAmbientLight);

		connect(mAmbientLight.get(), &QtProperty::valueChanged, [=]()
		{
			float v = mAmbientLight->value.toFloat();
			mScene->setAmbientLightColor(osg::Vec3f(v, v, v));
		});
	}
}
