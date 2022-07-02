/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#pragma once

#include "SkyboltVis/SkyboltVisFwd.h"

#include <osg/ref_ptr>
#include <functional>

namespace skybolt {
namespace vis {

osg::ref_ptr<RenderOperation> createRenderOperationVisualization(const osg::ref_ptr<RenderOperation>& rop, const ShaderPrograms& registry);

osg::ref_ptr<RenderOperation> createRenderOperationFunction(std::function<void(const RenderContext&)> func);

} // namespace vis
} // namespace skybolt
