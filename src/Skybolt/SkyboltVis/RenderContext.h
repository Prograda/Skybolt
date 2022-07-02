/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#pragma once

#include "SkyboltVisFwd.h"
#include <osg/Vec2i>
#include <osg/Vec3>

namespace skybolt {
namespace vis {

struct RenderContext
{
	osg::Vec2i targetDimensions; //!< Size of the render target in pixels
};

struct CameraRenderContext : public RenderContext
{
	explicit CameraRenderContext(const Camera& camera) : camera(camera) {}

	const Camera& camera;
	osg::Vec3f lightDirection; //!< Direction to light
	float atmosphericDensity; //!< Atmospheric density at camera's location, in units of kg/m^3
};

} // namespace vis
} // namespace skybolt
