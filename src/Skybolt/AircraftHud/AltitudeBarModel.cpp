/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#include "AltitudeBarModel.h"
#include <SkyboltCommon/Math/MathUtility.h>

using namespace skybolt;

AltitudeBarModel::AltitudeBarModel(HudDrawer* drawer, const Parameters &params) :
	mDrawer(drawer),
	mParams(params)
{
}

void AltitudeBarModel::drawBar(float altitude)
{
	if (altitude > mParams.maxAltitude)
	{
		if (mParams.hideBarWhenExceeded)
			return;
		altitude = mParams.maxAltitude;
	}
	float height = mParams.height * altitude / mParams.maxAltitude;
	mDrawer->drawSolidBox(mParams.position + glm::vec2(0, height*0.5f), mParams.width, height);
}

void AltitudeBarModel::drawMarker(float markerAltitude)
{
	markerAltitude = math::clamp(markerAltitude, 0.0f, mParams.maxAltitude);
	float x = mParams.position.x - mParams.width * 0.5f;
	float y = mParams.position.y + (markerAltitude / mParams.maxAltitude) * mParams.height;
	mDrawer->drawLine(glm::vec2(x - mParams.markerWidth, y - mParams.markerWidth),
					  glm::vec2(x, y));
	mDrawer->drawLine(glm::vec2(x, y),
		              glm::vec2(x - mParams.markerWidth, y + mParams.markerWidth));
}

void AltitudeBarModel::draw(float altitude, const float* markerAltitude)
{
	drawBar(altitude);

	if (markerAltitude)
		drawMarker(*markerAltitude);

	float oneOnMaxAlt = 1.0f / mParams.maxAltitude;
	for (size_t i=0; i < mParams.ticks.size(); i++)
	{
		float y = mParams.position.y + mParams.height * mParams.ticks[i] * oneOnMaxAlt;
		float x0 = mParams.position.x - (mParams.tickWidth * 0.5f + mParams.tickOffset);
		float x1 = x0 + mParams.tickWidth;
		mDrawer->drawLine(glm::vec2(x0, y), glm::vec2(x1, y));
	}
}
