/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltVis/SkyboltVisFwd.h"
#include "SkyboltVis/DefaultRootNode.h"
#include "SkyboltVis/OsgBox2.h"
#include <px_sched/px_sched.h>
#include <osg/Texture2D>
#include <atomic>
#include <functional>

namespace skybolt {
namespace vis {

typedef std::function<osg::Vec2f(const osg::Vec2f&)> Vec2Transform;

class GpuForestTile : public DefaultRootNode
{
public:
	GpuForestTile(const osg::ref_ptr<osg::Texture2D>& heightMap, const osg::ref_ptr<osg::Texture2D>& attributeMap, const std::shared_ptr<BillboardForest>& forest, const osg::Vec2f& tileWorldSizeMeters);

	void updatePreRender(const RenderContext& context) override;

private:
	osg::Uniform* mModelMatrixUniform;
};

} // namespace vis
} // namespace skybolt
