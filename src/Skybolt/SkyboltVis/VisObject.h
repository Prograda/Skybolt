/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltVisFwd.h"
#include <osg/Group>
#include <osg/Vec3f>
#include <osg/Quat>

namespace skybolt {
namespace vis {

class VisObject
{
public:
	VisObject();
	virtual ~VisObject();

	virtual void updatePreRender(const CameraRenderContext& context) {};

	virtual void setVisibilityCategoryMask(uint32_t mask) {};
	virtual void setVisible(bool visible) {};
	virtual bool isVisible() const {return true;}

	virtual osg::Node* _getNode() const = 0;
};

} // namespace vis
} // namespace skybolt
