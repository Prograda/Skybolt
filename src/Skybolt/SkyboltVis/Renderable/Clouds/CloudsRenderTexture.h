/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltVis/RenderOperation/RenderTexture.h"
#include <osg/PrimitiveSet>
#include <osg/Texture2D>

namespace skybolt {
namespace vis {

osg::ref_ptr<RenderTexture> createCloudsRenderTexture(const ScenePtr& scene);

} // namespace vis
} // namespace skybolt
