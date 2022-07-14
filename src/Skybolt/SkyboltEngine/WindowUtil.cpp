/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "WindowUtil.h"
#include <SkyboltEngine/EngineRoot.h>
#include <SkyboltEngine/EngineSettings.h>
#include <SkyboltVis/Camera.h>
#include <SkyboltVis/RenderOperation/DefaultRenderCameraViewport.h>
#include <SkyboltVis/Window/Window.h>

namespace skybolt {

osg::ref_ptr<vis::RenderCameraViewport> createAndAddViewportToWindowWithEngine(vis::Window& window, const EngineRoot& engineRoot)
{
	osg::ref_ptr<vis::RenderCameraViewport> viewport = new vis::DefaultRenderCameraViewport([&]{
		vis::DefaultRenderCameraViewportConfig c;
		c.scene = engineRoot.scene;
		c.programs = &engineRoot.programs;
		c.shadowParams = getShadowParams(engineRoot.engineSettings);
		c.cloudRenderingParams = getCloudRenderingParams(engineRoot.engineSettings);
		return c;
	}());
	window.getRenderOperationSequence().addOperation(viewport);
	return viewport;
}

} // namespace skybolt
