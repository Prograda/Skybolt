/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#include "HeadingRibbonModel.h"
#include <SkyboltCommon/Math/MathUtility.h>

using namespace skybolt;

HeadingRibbonModel::HeadingRibbonModel(HudDrawer* drawer, const Parameters &params) :
	mDrawer(drawer),
	mParams(params)
{
}

void HeadingRibbonModel::draw(float refHeadingDeg, float* markerHeadingDeg)
{
	// Make sure heading is in 0-360 degree range
	refHeadingDeg += 360.0f;
	refHeadingDeg = fmodf(refHeadingDeg, 360.0f);

	{	// Draw ticks
		float first = refHeadingDeg - mParams.headingRangeDegrees * 0.5f;
		first = std::ceil(first / mParams.minorIncrementDegrees) * mParams.minorIncrementDegrees; // snap to nearest tick on right

		float last = first + mParams.headingRangeDegrees;

		for (float val = first; val <= last; val+= mParams.minorIncrementDegrees)
		{
			float heading = val + 360.0f;
			heading = round(fmodf(heading, 360.0f));
			float height;
			std::string label;
			if (isMajorIncrement(heading))
			{
				height = mParams.majorHeight;
				label = getTickLabel(int(std::round(heading)));
			}
			else
			{
				height =  mParams.minorHeight;
			}
			drawTick(height, label, refHeadingDeg, val);
		}
	}

	// Draw waypoint marker
	if (markerHeadingDeg)
		drawMarker(refHeadingDeg, *markerHeadingDeg);

	// Draw current heading marker
	mDrawer->drawLine(glm::vec2(mParams.center.x, mParams.center.y),
					  glm::vec2(mParams.center.x, mParams.center.y - mParams.markerHeight));
}

bool HeadingRibbonModel::isMajorIncrement(float val)
{
	return (fmodf(val, mParams.majorIncrementDegrees) == 0.0f);
}

std::string HeadingRibbonModel::getTickLabel(int heading)
{
	if (mParams.useLettersForNSEW)
	{
		if (heading == 0)
			return "N";
		else if (heading == 90)
			return "E";
		else if (heading == 180)
			return "S";
		else if (heading == 270)
			return "W";
	}

	return std::to_string(heading);
}

float HeadingRibbonModel::getXForHeading(float refHeading, float heading)
{
	return mParams.center.x + (heading - refHeading) * (mParams.rangeWidth / mParams.headingRangeDegrees);
}

void HeadingRibbonModel::drawTick(float height, const std::string &label, float refHeading, float tickHeading)
{
	float x = getXForHeading(refHeading, tickHeading);
	float yStart = mParams.center.y + 0.5f * (mParams.majorHeight - height);
	mDrawer->drawLine( glm::vec2(x, yStart),
					   glm::vec2(x, yStart + height) );

	if (!label.empty())
		mDrawer->drawText(glm::vec2(x, mParams.center.y + (mParams.majorHeight + mParams.textGap)), label, 0, -1.0, HudDrawer::Alignment::Center);

}

float HeadingRibbonModel::clampXToHeadingRange(float x)
{
	return math::clamp(x, mParams.center.x - mParams.rangeWidth * 0.5f, mParams.center.x + mParams.rangeWidth * 0.5f);
}

void HeadingRibbonModel::drawMarker(float refHeading, float markerHeading)
{
	float x = getXForHeading(refHeading, markerHeading);
	x = clampXToHeadingRange(x);

	for (int dir = -1; dir <= 1; dir+=2)
	{
		mDrawer->drawLine( glm::vec2(x, mParams.center.y),
						   glm::vec2(x + (float)dir * mParams.markerHeight, mParams.center.y - mParams.markerHeight) );
	}
}
