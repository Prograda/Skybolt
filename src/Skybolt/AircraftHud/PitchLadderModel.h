/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#pragma once

#include "HudDrawer.h"

namespace skybolt {

class PitchLadderModel
{
public:
	PitchLadderModel(HudDrawer* drawer, float pitchAngleIncrement, float pitchGapHeight, float lineWidth, float wingletHeight, float textGap, float maxPitchAngle = skybolt::math::halfPiF());
	void draw(float pitch, float roll);

protected:
	void drawRung(float rungPitch, float pitch, float roll, float width, const HudDrawer::DashedLineParams* params = 0);
	void drawHalfRung(float relY, float rungPitch, float roll, float signedWidth, const HudDrawer::DashedLineParams* params = 0);

private:

	HudDrawer* mDrawer;
	float mPitchAngleIncrement;
	float mPitchGapHeight;
	float mLineWidth;
	float mWingletHeight;
	float mTextGap;
	float mMaxPitchAngle;
	HudDrawer::DashedLineParams mDashedLineParams;
};

} // namespace skybolt
