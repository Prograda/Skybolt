/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltVis/OsgBox2.h"
#include "SkyboltVis/SkyboltVisFwd.h"

#include <osg/MatrixTransform>
#include <osg/ref_ptr>

namespace skybolt {
namespace vis {

struct OsgTile
{
	osg::ref_ptr<osg::MatrixTransform> transform;
	TerrainPtr highResTerrain; //!< optional
	osg::Uniform* modelMatrixUniform;
};

} // namespace vis
} // namespace skybolt
