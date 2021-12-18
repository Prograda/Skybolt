/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "HelpDisplaySystem.h"
#include "SkyboltEngine/VisHud.h"
#include <SkyboltVis/Scene.h>
#include <SkyboltVis/Window/Window.h>
#include <osgViewer/Viewer>

namespace skybolt {

HelpDisplaySystem::HelpDisplaySystem(const vis::Window& window)
{
	osgViewer::Viewer& viewer = window.getViewer();
	mCamera = window.getRenderTargets().back().target->getOsgCamera();

	mHud = osg::ref_ptr<VisHud>(new VisHud());
	mCamera->addChild(mHud);
}

HelpDisplaySystem::~HelpDisplaySystem()
{
	setVisible(false);
}

void HelpDisplaySystem::setMessage(const std::string& message)
{
	const float lineHeight = 0.05f;
	const float textSize = lineHeight * 0.8f;

	mHud->clear();
	mHud->drawText(glm::vec2(-0.9f, 0.9f), message, 0.0f, textSize);
}

void HelpDisplaySystem::setVisible(bool visible)
{
	bool currentlyVisible = isVisible();
	if (visible && !currentlyVisible)
	{
		mCamera->addChild(mHud);
	}
	else if (!visible && currentlyVisible)
	{
		mCamera->removeChild(mHud);
	}
}

bool HelpDisplaySystem::isVisible()
{
	return (mHud->getNumParents() > 0);
}

void HelpDisplaySystem::updatePostDynamics(const System::StepArgs& args)
{
	osg::Viewport* viewport = mCamera->getViewport();
	mHud->setAspectRatio(viewport->width() / viewport->height());
}

} // namespace skybolt