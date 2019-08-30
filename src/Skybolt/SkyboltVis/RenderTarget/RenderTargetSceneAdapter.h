/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#pragma once

#include "SkyboltVis/SkyboltVisFwd.h"
#include "SkyboltVis/RenderContext.h"
#include "SkyboltVis/RenderTarget/RenderTarget.h"

#include <osg/Camera>
#include <osg/Group>

namespace skybolt {
namespace vis {

class RenderTargetSceneAdapter : public RenderTarget::Scene
{
public:
	RenderTargetSceneAdapter(const ScenePtr& scene) : m_scene(scene) {}
	~RenderTargetSceneAdapter() override {}
	void updatePreRender(const Camera& camera) override;

	osg::ref_ptr<osg::Node> getNode() override;

private:
	ScenePtr m_scene;
};

} // namespace vis
} // namespace skybolt
