/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "RenderTargetSceneAdapter.h"
#include "SkyboltVis/Camera.h"
#include "SkyboltVis/Scene.h"

namespace skybolt {
namespace vis {

void RenderTargetSceneAdapter::updatePreRender(const Camera& camera)
{
	RenderContext renderContext(camera);
	renderContext.lightDirection = -m_scene->getPrimaryLightDirection();
	renderContext.atmosphericDensity = m_scene->calcAtmosphericDensity(camera.getPosition());

	m_scene->updatePreRender(renderContext);
}

osg::ref_ptr<osg::Node> RenderTargetSceneAdapter::getNode()
{
	return m_scene->_getNode();
}

} // namespace vis
} // namespace skybolt
