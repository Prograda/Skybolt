/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#pragma once

#include "DefaultRootNode.h"

#include <osg/Light>

namespace skybolt {
namespace vis {

class Light : public DefaultRootNode
{
public:
	Light(const osg::Vec3f& direction = osg::Vec3f(1,0,0));
	~Light();

	osg::Vec3f getWorldLightDirection() const;

private:
	osg::LightSource* mLightSource;
};

} // namespace vis
} // namespace skybolt

