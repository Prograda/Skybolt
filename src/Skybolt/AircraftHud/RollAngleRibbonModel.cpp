/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#include "RollAngleRibbonModel.h"
#include <SkyboltCommon/Math/MathUtility.h>

using namespace skybolt;

RollAngleRibbonModel::RollAngleRibbonModel(HudDrawer* drawer, const Parameters &params) :
	mDrawer(drawer),
	mParams(params)
{
}

void RollAngleRibbonModel::drawTick(float angle, float length)
{
	angle *= skybolt::math::degToRadF();
	float r = mParams.lineRadius + glm::sign(length) * (mParams.majorLineLength - abs(length)) * 0.5f;
	glm::vec2 p0(r * glm::sin(angle), mParams.centerVerticalPosition + r * glm::cos(angle));
	r += length;
	glm::vec2 p1(r * glm::sin(angle), mParams.centerVerticalPosition + r * glm::cos(angle));

	mDrawer->drawLine(p0, p1);
}

bool RollAngleRibbonModel::isMajorTick(float angleDeg)
{
	return (fmodf(angleDeg, mParams.majorIncrementDeg) == 0);
}

void RollAngleRibbonModel::draw(float angleDeg)
{
	
	for (float angle = -mParams.angleLimitDeg; angle <= mParams.angleLimitDeg; angle += mParams.minorIncrementDeg)
	{
		float length = isMajorTick(angle) ? mParams.majorLineLength : mParams.minorLineLength;
		drawTick(angle, length);
	}

	// draw marker
	angleDeg = math::clamp(angleDeg, -mParams.angleLimitDeg, mParams.angleLimitDeg);
	drawTick(angleDeg, -mParams.markerLineLength);
}