/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#include "Light.h"
#include "Scene.h"

#include <osg/LightSource>

using namespace skybolt::vis;

Light::Light(const osg::Vec3f& direction)
{
	osg::Light* light = new osg::Light;
	light->setDirection(direction);

	mLightSource = new osg::LightSource;
	mLightSource->setLight(light);

	mTransform->addChild(mLightSource);
}

Light::~Light()
{
	mTransform->removeChild(mLightSource);
}

osg::Vec3f Light::getWorldLightDirection() const
{
	return getOrientation() * mLightSource->getLight()->getDirection();
}
