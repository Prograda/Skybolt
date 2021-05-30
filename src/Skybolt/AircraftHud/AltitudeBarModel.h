/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#pragma once

#include "HudDrawer.h"
#include <vector>

namespace skybolt {

class AltitudeBarModel
{
public:

	struct Parameters
	{
		void makeDefault()
		{
			maxAltitude = 200;
			position = glm::vec2(0.4f, -0.3f);
			height = 0.5f;
			width = 0.01f;
			markerWidth = 0.008f;
			tickWidth = 0.018f;
			tickOffset = 0.003f;
			hideBarWhenExceeded = true;

			for (int i = 0; i < 5; i++)
				ticks.push_back(20.0f*(float)i);
			for (int i = 0; i < 5; i++)
				ticks.push_back(50.0f*(float)i);
		}

		float maxAltitude;
		glm::vec2 position;
		float height;
		float width;
		std::vector<float> ticks;
		float markerWidth;
		float tickWidth;
		float tickOffset;
		bool hideBarWhenExceeded;
	};

	AltitudeBarModel(HudDrawer* drawer, const Parameters &params);
	// marker is optional
	void draw(float altitude, const float* markerAltitude = 0);

	const Parameters &getParameters() { return mParams; }

private:
	void drawBar(float altitude);
	void drawMarker(float markerAltitude);

	HudDrawer* mDrawer;
	Parameters mParams;
};

} // namespace skybolt
