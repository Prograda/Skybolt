/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <SkyboltCommon/NonNullPtr.h>
#include <SkyboltEngine/SkyboltEngineFwd.h>
#include <SkyboltVisOsg/SkyboltVisOsgFwd.h>

#include <nlohmann/json.hpp>
#include <osg/ref_ptr>

namespace skybolt {

struct VisContext
{
	vis::ScenePtr scene;
	NonNullPtr<const vis::ShaderPrograms> programs;
	nlohmann::json engineSettings;
};;

VisContext createVisContext(const vis::VisRoot& visRoot, const EngineRoot& engineRoot);

osg::ref_ptr<vis::RenderCameraViewport> createAndAddViewportToWindow(vis::Window& window, const VisContext& visContext);

} // namespace skybolt