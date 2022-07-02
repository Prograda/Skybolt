/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#pragma once

#include "RenderOperation.h"
#include "SkyboltVis/Rect.h"
#include "SkyboltVis/RenderContext.h"

#include <osg/Camera>
#include <osgViewer/Viewer>

namespace skybolt {
namespace vis {

//! Renders from a vis::Camera into a viewport region of the frame buffer
class RenderCameraViewport : public RenderOperation
{
public:
	virtual ~RenderCameraViewport() = default;
	
	virtual void setCamera(const CameraPtr& camera) { mCamera = camera; }
	virtual CameraPtr getCamera() const { return mCamera; }
	
	virtual osg::ref_ptr<RenderTarget> getFinalRenderTarget() const = 0;

private:
	CameraPtr mCamera;
};

} // namespace vis
} // namespace skybolt
