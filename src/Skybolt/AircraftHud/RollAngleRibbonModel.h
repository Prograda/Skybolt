/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#pragma once

#include "HudDrawer.h"

namespace skybolt {

class RollAngleRibbonModel
{
public:

	struct Parameters
	{
		void makeDefault()
		{
			angleLimitDeg = 30;
			majorIncrementDeg = 10;
			minorIncrementDeg = 5;
			majorLineLength = 0.01f;
			minorLineLength = 0.005f;
			markerLineLength = majorLineLength;
			lineRadius = 0.3f;
			centerVerticalPosition = 0.3;
		}

		float angleLimitDeg;
		float majorIncrementDeg;
		float minorIncrementDeg;
		float majorLineLength;
		float minorLineLength;
		float markerLineLength;
		float lineRadius;
		float centerVerticalPosition;
	};

	RollAngleRibbonModel(HudDrawer* drawer, const Parameters &params);
	void draw(float angleDeg);

private:
	void drawTick(float angle, float length);
	bool isMajorTick(float angleDeg);

	HudDrawer* mDrawer;
	Parameters mParams;
};

} // namespace skybolt
