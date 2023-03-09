/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#include "ZenithCompassModel.h"
#include <SkyboltCommon/Math/MathUtility.h>

using namespace skybolt;

ZenithCompassModel::ZenithCompassModel(HudDrawer* drawer, const Parameters &params) :
	mDrawer(drawer),
	mParams(params)
{
}

void ZenithCompassModel::draw(const glm::vec2 &center, float rotation)
{
	// Draw concentric circles
	float minPitch = mParams.minPitchAngle - 0.001f;
	for (float angle = math::halfPiF() - mParams.pitchAngleIncrement; angle >= minPitch; angle -= mParams.pitchAngleIncrement)
	{
		float radius = (math::halfPiF() - angle) *  (mParams.pitchDistPerIncrement / mParams.pitchAngleIncrement);
		mDrawer->drawCircle(center, radius, 64);
	}

	// Draw center cross
	float outerRadius = (mParams.pitchDistPerIncrement / mParams.pitchAngleIncrement) * (math::halfPiF() - mParams.minPitchAngle);
	float crossRadius = 0.2f * outerRadius;
	{
		glm::vec2 v = glm::vec2(0, crossRadius);
		glm::vec2 p0 = center + math::vec2Rotate(v, rotation);
		glm::vec2 p1 = center + math::vec2Rotate(v, rotation + math::piF());
		glm::vec2 p2 = center + math::vec2Rotate(v, rotation + math::halfPiF());
		glm::vec2 p3 = center + math::vec2Rotate(v, rotation - math::halfPiF());

		mDrawer->drawLine(p0, p1);
		mDrawer->drawLine(p2, p3);
	}

	// Draw heading ticks
	for (int angle = 0; angle < 360; angle += 90)
	{
		float angleRad = skybolt::math::degToRadF() * (float)angle + rotation;

		glm::vec2 dir = math::vec2Rotate(glm::vec2(0, 1), angleRad);

		glm::vec2 p0 = center + dir * (outerRadius - 0.5f * mParams.headingTickLength);
		glm::vec2 p1 = center + dir * (outerRadius + 0.5f * mParams.headingTickLength);
		glm::vec2 p2 = center + dir * (outerRadius + 0.5f * mParams.headingTickLength + mParams.textGap);

		mDrawer->drawLine(p0, p1);
		mDrawer->drawText(p2, getTickLabel((float)angle), 0.0f, -1.0, HudDrawer::Alignment::Center);
	}
}

std::string ZenithCompassModel::getTickLabel(float heading)
{
	if (mParams.useLettersForNSEW)
	{
		if (heading == 0.0f)
			return "N";
		else if (heading == 90.0f)
			return "E";
		else if (heading == 180.0f)
			return "S";
		else if (heading == 270.0f)
			return "W";
	}
	int val = (int)std::floor(heading);
	return std::to_string(val);
}
