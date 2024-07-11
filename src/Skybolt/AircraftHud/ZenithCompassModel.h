/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#pragma once

#include "HudDrawer.h"

namespace skybolt {

class ZenithCompassModel
{
public:
	struct Parameters
	{
		float pitchAngleIncrement;
		float pitchDistPerIncrement;
		float minPitchAngle;
		float textGap;
		float headingTickLength;
		bool useLettersForNSEW;
	};

	ZenithCompassModel(HudDrawer* drawer, const Parameters &params);
	// angle in radians
	void draw(const glm::vec2 &center, float angle);

private:
	std::string getTickLabel(float heading);

	HudDrawer* mDrawer;
	Parameters mParams;
};

} // namespace skybolt
