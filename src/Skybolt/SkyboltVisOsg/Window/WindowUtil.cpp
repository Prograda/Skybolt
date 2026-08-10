/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "WindowUtil.h"
#include <SkyboltEngine/EngineRoot.h>
#include <SkyboltEngine/EngineSettings.h>
#include <SkyboltVisOsg/Camera.h>
#include <SkyboltVisOsg/VisRoot.h>
#include <SkyboltVisOsg/RenderOperation/DefaultRenderCameraViewport.h>
#include <SkyboltVisOsg/Window/Window.h>

namespace skybolt {

VisContext createVisContext(const vis::VisRoot& visRoot, const EngineRoot& engineRoot)
{
	return VisContext{
		.scene = visRoot.getScene(),
		.programs = &visRoot.getShaderPrograms(),
		.engineSettings = engineRoot.engineSettings
	};
}

osg::ref_ptr<vis::RenderCameraViewport> createAndAddViewportToWindow(vis::Window& window, const VisContext& visContext)
{
	osg::ref_ptr<vis::RenderCameraViewport> viewport = new vis::DefaultRenderCameraViewport([&]{
		vis::DefaultRenderCameraViewportConfig c;
		c.scene = visContext.scene;
		c.programs = visContext.programs;
		c.shadowParams = getShadowParams(visContext.engineSettings);
		c.cloudRenderingParams = getCloudRenderingParams(visContext.engineSettings);
		return c;
	}());
	window.getRenderOperationSequence().addOperation(viewport);
	return viewport;
}

} // namespace skybolt
