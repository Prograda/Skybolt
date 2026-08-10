/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "HeightMapElevationProvider.h"
#include "SkyboltVis/Image/Image.h"
#include <SkyboltCommon/Math/MathUtility.h>

namespace skybolt {
namespace vis {

HeightMapElevationProvider::HeightMapElevationProvider(const ImagePtr& image, const ElevationRerange& elevationRerange, const Box2d& bounds) :
	mImage(image),
	mElevationRerange(elevationRerange),
	mHorizontalOffset(bounds.minimum.y, bounds.minimum.x)
{
	mHorizontalScale = glm::dvec2(
		image->getWidth() / (bounds.maximum.y - bounds.minimum.y),
		image->getHeight() / (bounds.maximum.x - bounds.minimum.x));
}

float HeightMapElevationProvider::get(double x, double y) const
{
	glm::dvec3 uv = glm::dvec3((y - mHorizontalOffset.x) * mHorizontalScale.x,
		(x - mHorizontalOffset.y) * mHorizontalScale.y, 0.0f);

	int sMax = mImage->getWidth() - 1;
	int tMax = mImage->getHeight() - 1;

	uv.x = skybolt::math::clamp(uv.x, 0.0, double(sMax));
	uv.y = skybolt::math::clamp(uv.y, 0.0, double(tMax));

	int u0 = (int)uv.x;
	int u1 = std::min(u0 + 1, sMax);
	int v0 = (int)uv.y;
	int v1 = std::min(v0 + 1, tMax);

	float fracU = float(uv.x) - float(u0);
	float fracV = float(uv.y) - float(v0);

	const uint16_t* ptr = (const uint16_t*)mImage->getRawData();

	int width = mImage->getWidth();
	float d00 = float(ptr[u0 + width * v0]);
	float d10 = float(ptr[u1 + width * v0]);
	float d01 = float(ptr[u0 + width * v1]);
	float d11 = float(ptr[u1 + width * v1]);

	float d0 = skybolt::math::lerp(d00, d10, fracU);
	float d1 = skybolt::math::lerp(d01, d11, fracU);

	return getElevationForColorValue(mElevationRerange, int(std::round(skybolt::math::lerp(d0, d1, fracV))));
}

} // namespace vis
} // namespace skybolt
