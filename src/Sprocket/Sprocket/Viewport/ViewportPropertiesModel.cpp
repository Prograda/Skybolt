#include "ViewportPropertiesModel.h"
#include <SkyboltCommon/Math/MathUtility.h>
#include <SkyboltSim/Components/CameraComponent.h>
#include <SkyboltVis/Scene.h>

using namespace skybolt;

ViewportPropertiesModel::ViewportPropertiesModel(vis::Scene* Scene, sim::CameraComponent* camera) :
	mScene(Scene)
{
	{
		mFov = createVariantProperty("Vertical FOV", 0.0);
		mFov->setValue(camera->getState().fovY * skybolt::math::radToDegF());
		mProperties.push_back(mFov);

		connect(mFov.get(), &VariantProperty::valueChanged, [=]()
		{
			camera->getState().fovY = mFov->value.toFloat() * skybolt::math::degToRadF();
		});
	}
	{
		mAmbientLight = createVariantProperty("Ambient Light", 0.0);
		mAmbientLight->setValue(mScene->getAmbientLightColor().x());
		mProperties.push_back(mAmbientLight);

		connect(mAmbientLight.get(), &VariantProperty::valueChanged, [=]()
		{
			float v = mAmbientLight->value.toFloat();
			mScene->setAmbientLightColor(osg::Vec3f(v, v, v));
		});
	}
}
