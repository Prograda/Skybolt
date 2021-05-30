/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#pragma once

#include "HudDrawer.h"

namespace skybolt {

class HudHoverVelModel
{
public:
	HudHoverVelModel(HudDrawer* drawer, float speedScale, float markerRadius);

	//! @param vel x is forward speed, y is right speed
	void draw(const glm::vec2 &vel);

private:
	HudDrawer* mDrawer;
	float mSpeedScale;
	float mMarkerRadius;
};

} // namespace skybolt
