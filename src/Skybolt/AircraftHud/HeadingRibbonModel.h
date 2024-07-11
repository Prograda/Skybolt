/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#pragma once

#include "HudDrawer.h"

namespace skybolt {

class HeadingRibbonModel
{
public:

	struct Parameters
	{
		void makeDefault()
		{
			center = glm::vec2(0,0);
			rangeWidth = 0.7f;
			headingRangeDegrees = 180.0f;
			majorIncrementDegrees = 30.0f;
			minorIncrementDegrees = 10.0f;
			majorHeight = 0.02f;
			minorHeight = 0.01f;
			markerHeight = minorHeight;
			textGap = 0.01f;
			useLettersForNSEW = true;
		}

		glm::vec2 center;
		float rangeWidth;
		float headingRangeDegrees;
		float majorIncrementDegrees;
		float minorIncrementDegrees;
		float majorHeight;
		float minorHeight;
		float markerHeight;
		float textGap;
		bool useLettersForNSEW;
	};

	HeadingRibbonModel(HudDrawer* drawer, const Parameters &params);
	// Heading in degrees. Marker heading is optional
	void draw(float refHeadingDeg, float* markerHeadingDeg = 0);

protected:
	void drawTick(float height, const std::string &label, float refHeading, float tickHeading);
	
	void drawMarker(float refHeading, float markerHeading);

	float getXForHeading(float refHeading, float heading);
	float clampXToHeadingRange(float x);

	bool isMajorIncrement(float val);
	std::string getTickLabel(int heading);

private:
	HudDrawer* mDrawer;
	Parameters mParams;
};

} // namespace skybolt
