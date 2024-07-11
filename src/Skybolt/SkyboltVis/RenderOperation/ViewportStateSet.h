/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltVis/SkyboltVisFwd.h"
#include <osg/StateSet>

namespace skybolt {
namespace vis {

class ViewportStateSet : public osg::StateSet
{
public:
	ViewportStateSet();

	void update(const vis::Camera& camera);

private:
	osg::Uniform* mCameraPositionUniform;
	osg::Uniform* mCameraCenterDirectionUniform;
	osg::Uniform* mCameraUpDirectionUniform;
	osg::Uniform* mCameraRightDirectionUniform;
	osg::Uniform* mFarClipDistanceUniform;
	osg::Uniform* mViewMatrixUniform;
	osg::Uniform* mViewProjectionMatrixUniform;
	osg::Uniform* mProjectionMatrixUniform;
};

} // namespace vis
} // namespace skybolt
