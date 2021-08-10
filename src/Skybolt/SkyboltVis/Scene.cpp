/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#include "Scene.h"
#include "Camera.h"
#include "Light.h"
#include "RenderContext.h"
#include "VisObject.h"
#include "Renderable/Planet/Planet.h"
#include <SkyboltCommon/VectorUtility.h>

#include <osg/ClipNode>

using namespace skybolt::vis;

Scene::Scene() :
	mGroup(new osg::Group),
	mClipNode(new osg::ClipNode),
	mPrimaryLight(nullptr),
	mPrimaryPlanet(nullptr),
	mWrappedNoisePeriod(10000.f)
{
	mClipNode->addChild(mGroup);

	osg::StateSet* ss = mGroup->getOrCreateStateSet();
	mCameraPositionUniform = new osg::Uniform("cameraPosition", osg::Vec3f(0, 0, 0));
	ss->addUniform(mCameraPositionUniform);

	mViewCameraPositionUniform = new osg::Uniform("viewCameraPosition", osg::Vec3f(0, 0, 0));
	ss->addUniform(mViewCameraPositionUniform);

	mCameraCenterDirectionUniform = new osg::Uniform("cameraCenterDirection", osg::Vec3f(0, 0, 0));
	ss->addUniform(mCameraCenterDirectionUniform);

	mCameraUpDirectionUniform = new osg::Uniform("cameraUpDirection", osg::Vec3f(0, 0, 0));
	ss->addUniform(mCameraUpDirectionUniform);

	mCameraRightDirectionUniform = new osg::Uniform("cameraRightDirection", osg::Vec3f(0, 0, 0));
	ss->addUniform(mCameraRightDirectionUniform);

	mViewMatrixUniform = new osg::Uniform("viewMatrix", osg::Matrixf());
	ss->addUniform(mViewMatrixUniform);

	mViewProjectionMatrixUniform = new osg::Uniform("viewProjectionMatrix", osg::Matrixf());
	ss->addUniform(mViewProjectionMatrixUniform);

	mLightDirectionUniform = new osg::Uniform("lightDirection", osg::Vec3f(0, 0, -1));
	ss->addUniform(mLightDirectionUniform);

	mDirectLightColorUniform = new osg::Uniform("directLightColor", osg::Vec3f(1, 1, 1));
	ss->addUniform(mDirectLightColorUniform);

	mAmbientLightColorUniform = new osg::Uniform("ambientLightColor", osg::Vec3f(0.0f, 0.0f, 0.0f));
	ss->addUniform(mAmbientLightColorUniform);

	mWrappedNoiseOriginUniform = new osg::Uniform("wrappedNoiseOrigin", osg::Vec3f());
	ss->addUniform(mWrappedNoiseOriginUniform);

	ss->addUniform(new osg::Uniform("wrappedNoisePeriod", mWrappedNoisePeriod));

	mGroundIrradianceMultiplierUniform = new osg::Uniform("groundIrradianceMultiplier", 0.f);
	ss->addUniform(mGroundIrradianceMultiplierUniform);
}

Scene::~Scene()
{}

void Scene::updatePreRender(const RenderContext& context)
{
	mCameraPositionUniform->set(osg::Vec3f(context.camera.getPosition()));
	mViewCameraPositionUniform->set(osg::Vec3f(context.camera.getPosition()));

	mCameraCenterDirectionUniform->set(context.camera.getOrientation() * osg::Vec3f(1, 0, 0));
	mCameraUpDirectionUniform->set(context.camera.getOrientation() * osg::Vec3f(0, 0, -1));
	mCameraRightDirectionUniform->set(context.camera.getOrientation() * osg::Vec3f(0, 1, 0));

	const auto& camera = context.camera;

	mViewMatrixUniform->set(camera.getViewMatrix());

	osg::Matrixf viewProj = camera.getViewMatrix() * camera.getProjectionMatrix();
	osg::Matrixf viewProjInv = osg::Matrix::inverse(viewProj);
	mViewProjectionMatrixUniform->set(viewProj);

	mLightDirectionUniform->set(-getPrimaryLightDirection());

	mWrappedNoiseOriginUniform->set(mWrappedNoiseOrigin);

	{
		float multiplier = 0.f;
		if (mPrimaryPlanet)
		{
			float planetRadius = float(mPrimaryPlanet->getInnerRadius());
			float altitude = float((context.camera.getPosition() - mPrimaryPlanet->getPosition()).length()) - planetRadius;
			multiplier = glm::clamp(glm::mix(1.f, 0.f, altitude / planetRadius), 0.f, 1.f);
		}
		mGroundIrradianceMultiplierUniform->set(multiplier);
	}

	for (VisObject* object : mObjects)
	{
		object->updatePostSceneUpdate();
	}

	for (VisObject* object : mObjects)
	{
		object->updatePreRender(context);
	}
}

void Scene::addObject(VisObject* object)
{
	mObjects.push_back(object);
	mGroup->addChild(object->_getNode());

	if (Light* light = dynamic_cast<Light*>(object))
	{
		mPrimaryLight = light;
	}
	else if (Planet* planet = dynamic_cast<Planet*>(object))
	{
		mPrimaryPlanet = planet;
	}
}

void Scene::removeObject(VisObject* object)
{
	if (object == mPrimaryLight)
		mPrimaryLight = 0;
	else if (object == mPrimaryPlanet)
		mPrimaryPlanet = 0;

	mGroup->removeChild(object->_getNode());
	skybolt::VectorUtility::eraseFirst(mObjects, object);
}

void Scene::addClipPlane(osg::ClipPlane* plane)
{
	mClipNode->addClipPlane(plane);
}

void Scene::removeClipPlane(osg::ClipPlane* plane)
{
	mClipNode->removeClipPlane(plane);
}

float Scene::calcAtmosphericDensity(const osg::Vec3f& position) const
{
	return mPrimaryPlanet ? mPrimaryPlanet->calcAtmosphericDensity(position) : 0.0f;
}

osg::Vec3f Scene::getPrimaryLightDirection() const
{
	if (mPrimaryLight)
		return mPrimaryLight->getWorldLightDirection();
	else
		return osg::Vec3f(0,0,-1);
}

void Scene::translateNoiseOrigin(const osg::Vec3f& offset)
{
	mWrappedNoiseOrigin += offset;
	for (int i = 0; i < 3; ++i)
		mWrappedNoiseOrigin[i] = fmodf(mWrappedNoiseOrigin[i], mWrappedNoisePeriod);
}

osg::Vec3f Scene::getAmbientLightColor() const
{
	osg::Vec3f v;
	mAmbientLightColorUniform->get(v);
	return v;
}

void Scene::setAmbientLightColor(const osg::Vec3f& color)
{
	mAmbientLightColorUniform->set(color);
}
