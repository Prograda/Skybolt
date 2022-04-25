/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#pragma once

#include "VisObject.h"

namespace skybolt {
namespace vis {

class RootNode : public VisObject
{
public:
	virtual void setPosition(const osg::Vec3d &position) = 0;
	virtual void setOrientation(const osg::Quat &orientation) = 0;
	virtual void setTransform(const osg::Matrix& m) = 0;

	virtual osg::Vec3d getPosition() const = 0;
	virtual osg::Quat getOrientation() const = 0;
	virtual osg::Matrix getTransform() const = 0;
};

} // namespace vis
} // namespace skybolt

