/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "WindowUtility.h"
#include <SkyboltEngine/EngineRoot.h>
#include <SkyboltVis/RenderTarget/RenderTargetSceneAdapter.h>
#include <SkyboltVis/RenderTarget/Viewport.h>
#include <SkyboltVis/RenderTarget/ViewportHelpers.h>

namespace skybolt {

osg::ref_ptr<vis::RenderTarget> createAndAddViewportToWindowWithEngine(vis::Window& window, const EngineRoot& engineRoot)
{
	auto viewport = createAndAddViewportToWindow(window, engineRoot.programs.getRequiredProgram("compositeFinal"));
	viewport->setScene(std::make_shared<vis::RenderTargetSceneAdapter>(engineRoot.scene));
	return viewport;
}

} // namespace skybolt
