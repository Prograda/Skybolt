/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#pragma once

#include <SkyboltCommon/Math/MathUtility.h>
#include <string>

namespace skybolt {

class HudDrawer
{
public:

	struct DashedLineParams
	{
		 float dashLength;
		 float gapLength;
	};

	virtual void drawLine(const glm::vec2 &p0, const glm::vec2 &p1) = 0;
	virtual void drawLineDashed(const glm::vec2 &p0, const glm::vec2 &p1, const DashedLineParams& params) = 0;

	enum class Alignment
	{
		Left,
		Center,
		Right
	};

	/*!
		@param rotation is positive from X axis toward Y axis
		If size is < 0 then default size is used
	*/
	virtual void drawText(const glm::vec2 &p, const std::string &message, float rotation, float size = -1.0, Alignment alignment = Alignment::Left) = 0;
	//! @param position is center of box
	virtual void drawSolidBox(const glm::vec2 &position, float width, float height) = 0;
	
	virtual void drawCircle(const glm::vec2 &center, float radius, int segmentCount, float startAngle = 0, float angleRange = skybolt::math::twoPiF())
	{
		float angle = startAngle;
		float angleDelta = angleRange / (float)segmentCount;
		glm::vec2 p = center + radius * glm::vec2(cos(angle), sin(angle));
		for (int i = 1; i <= segmentCount; ++i)
		{
			angle += angleDelta;
			glm::vec2 p2 = center + radius * glm::vec2(cos(angle), sin(angle));
			drawLine(p, p2);
			p = p2;
		}
	}
};

} // namespace skybolt
