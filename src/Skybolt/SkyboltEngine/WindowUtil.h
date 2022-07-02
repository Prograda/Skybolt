/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <SkyboltEngine/SkyboltEngineFwd.h>
#include <SkyboltVis/SkyboltVisFwd.h>

#include <osg/ref_ptr>

namespace skybolt {

osg::ref_ptr<vis::RenderCameraViewport> createAndAddViewportToWindowWithEngine(vis::Window& window, const EngineRoot& engineRoot);

} // namespace skybolt