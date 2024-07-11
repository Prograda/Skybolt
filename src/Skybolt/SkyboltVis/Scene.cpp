/* Copyright Matthew Reid
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

using namespace skybolt::vis;

Scene::Scene(const osg::ref_ptr<osg::StateSet>& ss) :
	mStateSet(ss),
	mPrimaryLight(nullptr),
	mPrimaryPlanet(nullptr),
	mWrappedNoisePeriod(10000.f)
{
	assert(mStateSet);

	for (int i = 0; i < (int)Bucket::BucketCount; ++i)
	{
		mBucketGroups.push_back(new osg::Group);
	}

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

Scene::~Scene() = default;

void Scene::updatePreRender(const CameraRenderContext& context)
{
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

	for (auto& [object, bucket] : mObjects)
	{
		object->updatePreRender(context);
	}
}

void Scene::addObject(const VisObjectPtr& object, Bucket bucket)
{
	assert(mObjects.find(object) == mObjects.end());
	mObjects[object] = bucket;
	mBucketGroups[(int)bucket]->addChild(object->_getNode());

	if (Light* light = dynamic_cast<Light*>(object.get()))
	{
		mPrimaryLight = light;
	}
	else if (Planet* planet = dynamic_cast<Planet*>(object.get()))
	{
		mPrimaryPlanet = planet;
	}
}

void Scene::removeObject(const VisObjectPtr& object)
{
	if (object.get() == mPrimaryLight)
		mPrimaryLight = 0;
	else if (object.get() == mPrimaryPlanet)
		mPrimaryPlanet = 0;

	auto i = mObjects.find(object);
	if (i != mObjects.end())
	{
		mBucketGroups[(int)i->second]->removeChild(object->_getNode());
		mObjects.erase(i);
	}
}

float Scene::calcAtmosphericDensity(const osg::Vec3f& position) const
{
	return mPrimaryPlanet ? mPrimaryPlanet->calcAtmosphericDensity(position) : 0.0f;
}

osg::Group* Scene::getBucketGroup(Bucket bucket) const
{
	assert((std::size_t)bucket < mBucketGroups.size());
	return mBucketGroups[(int)bucket];
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
