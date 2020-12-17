/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#include "Viewport.h"
#include "Camera.h"
#include "SkyboltVis/Window/Window.h"

using namespace skybolt::vis;

Viewport::Viewport() :
	RenderTarget(new osg::Camera)
{
	mOsgCamera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
	mOsgCamera->setComputeNearFarMode(osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);
	mOsgCamera->setClearColor(osg::Vec4(0, 0, 0, 0));

	addChild(mOsgCamera);
	setRect(RectI(0, 0, 600, 400));
}
