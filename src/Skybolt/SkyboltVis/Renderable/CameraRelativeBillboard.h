/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#pragma once

#include "SkyboltVis/DefaultRootNode.h"
#include <osg/Billboard>

namespace skybolt {
namespace vis {

class CameraRelativeBillboard : public DefaultRootNode
{
public:
	CameraRelativeBillboard(const osg::ref_ptr<osg::StateSet>& stateSet, float width, float height, float distance);

	virtual void setPosition(const osg::Vec3d &position) override {} // has no effect

protected:
	void updatePreRender(const CameraRenderContext& context);

private:
	float mDistance;
};

} // namespace vis
} // namespace skybolt
