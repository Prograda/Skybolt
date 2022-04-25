/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <osg/StateSet>

namespace skybolt {
namespace vis {

enum class RenderBinId
{
	Stars = -3,
	CelestialBody = -2,
	Sky = -1
};

void setRenderBin(osg::StateSet& stateSet, RenderBinId bin);

} // namespace vis
} // namespace skybolt
