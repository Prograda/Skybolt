/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#include "HudHoverVelModel.h"

using namespace skybolt;

HudHoverVelModel::HudHoverVelModel(HudDrawer* drawer, float speedScale, float markerRadius) :
	mDrawer(drawer),
	mSpeedScale(speedScale),
	mMarkerRadius(markerRadius)
{
}


void HudHoverVelModel::draw(const glm::vec2 &vel)
{
	glm::vec2 markerPos = glm::vec2(vel.y, vel.x) * mSpeedScale;
	mDrawer->drawLine(glm::vec2(0.0f), markerPos);
	mDrawer->drawLine(glm::vec2(markerPos.x - mMarkerRadius, markerPos.y), glm::vec2(markerPos.x + mMarkerRadius, markerPos.y));
	mDrawer->drawLine(glm::vec2(markerPos.x, markerPos.y - mMarkerRadius), glm::vec2(markerPos.x, markerPos.y + mMarkerRadius));
}