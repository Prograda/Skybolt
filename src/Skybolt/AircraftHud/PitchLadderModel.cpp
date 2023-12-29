/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#include "PitchLadderModel.h"
#include <SkyboltCommon/Math/MathUtility.h>

using namespace skybolt;

PitchLadderModel::PitchLadderModel(HudDrawer* drawer, const Parameters& param) :
	mDrawer(drawer),
	mParam(param)
{
	int dashCount = 4;
	float gapFraction = 0.2f;
	float actualLineWidth = mParam.lineWidth * 0.25f; // drawn lines are a quater of the specified length
	mDashedLineParams.dashLength = (actualLineWidth) / ((dashCount-1) * (gapFraction+1) + 1); // determine perfect dash length for ending on a complete dash
	mDashedLineParams.gapLength = mDashedLineParams.dashLength * gapFraction;

}

void PitchLadderModel::drawHalfRung(float relY, float rungPitch, float roll, float signedWidth, const HudDrawer::DashedLineParams* params)
{
	float halfSignedWidth = signedWidth * 0.5f;
	glm::vec2 p0 = math::vec2Rotate( glm::vec2(halfSignedWidth, relY + glm::sign(rungPitch) * -mParam.wingletHeight), roll);
	glm::vec2 p1 = math::vec2Rotate( glm::vec2(halfSignedWidth, relY), roll);
	glm::vec2 p2 = math::vec2Rotate( glm::vec2(halfSignedWidth * 0.5, relY), roll);

	// Draw winglet
	mDrawer->drawLine(p0, p1);

	// horizontal line
	if (params)
		mDrawer->drawLineDashed(p1, p2, *params);
	else
		mDrawer->drawLine(p1, p2);

	glm::vec2 textPos(halfSignedWidth + glm::sign(halfSignedWidth) * mParam.textOffset, relY);
	int angleTextValue = int(round(skybolt::math::radToDegF() * rungPitch));
	if (angleTextValue > 90)
		angleTextValue = 180 - angleTextValue;
	mDrawer->drawText(math::vec2Rotate(textPos, roll), std::to_string(angleTextValue), roll, -1.0, HudDrawer::Alignment::Center);
}

void PitchLadderModel::drawRung(float rungPitch, float pitch, float roll, float width, const HudDrawer::DashedLineParams* params)
{
	float relY = (rungPitch - pitch) * mParam.pitchGapHeight / mParam.pitchAngleIncrement;

	if (isRungInDisplayArea(relY, roll))
	{
		drawHalfRung(relY, rungPitch, roll, -width, params);
		drawHalfRung(relY, rungPitch, roll, width, params);
	}
}

bool PitchLadderModel::isRungInDisplayArea(float relY, float roll) const
{
	glm::vec2 p = math::vec2Rotate( glm::vec2(0, relY), roll);
	glm::vec2 halfDisplayAreaSize = mParam.displayAreaSize * 0.5f;
	return p.x > -halfDisplayAreaSize.x && p.x < halfDisplayAreaSize.x
		&& p.y > -halfDisplayAreaSize.y && p.y < halfDisplayAreaSize.y;
}

void PitchLadderModel::draw(float pitch, float roll)
{
	drawRung(0, pitch, roll, mParam.lineWidth * 1.5f); // horizon

	for (float p = mParam.pitchAngleIncrement; p <= mParam.maxPitchAngle; p += mParam.pitchAngleIncrement)
		drawRung(p, pitch, roll, mParam.lineWidth);

	for (float p = -mParam.pitchAngleIncrement; p >= -mParam.maxPitchAngle; p -= mParam.pitchAngleIncrement)
		drawRung(p, pitch, roll, mParam.lineWidth, &mDashedLineParams);
}