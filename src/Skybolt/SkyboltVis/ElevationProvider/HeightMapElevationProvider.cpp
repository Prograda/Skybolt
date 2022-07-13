/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "HeightMapElevationProvider.h"
#include <SkyboltCommon/Math/MathUtility.h>

namespace skybolt {
namespace vis {

HeightMapElevationProvider::HeightMapElevationProvider(const osg::ref_ptr<osg::Image>& image, const HeightMapElevationRerange& elevationRerange, const Box2f& bounds) :
	mImage(image),
	mElevationRerange(elevationRerange),
	mHorizontalOffset(bounds.minimum.y(), bounds.minimum.x())
{
	mHorizontalScale = osg::Vec2f(image->s() / (bounds.maximum.y() - bounds.minimum.y()),
		image->t() / (bounds.maximum.x() - bounds.minimum.x()));
}

float HeightMapElevationProvider::get(float x, float y) const
{
	osg::Vec3f uv = osg::Vec3f((y - mHorizontalOffset.x()) * mHorizontalScale.x(),
		(x - mHorizontalOffset.y()) * mHorizontalScale.y(), 0.0f);

	int sMax = mImage->s() - 1;
	int tMax = mImage->t() - 1;

	uv.x() = skybolt::math::clamp(uv.x(), 0.0f, float(sMax));
	uv.y() = skybolt::math::clamp(uv.y(), 0.0f, float(tMax));

	int u0 = (int)uv.x();
	int u1 = std::min(u0 + 1, sMax);
	int v0 = (int)uv.y();
	int v1 = std::min(v0 + 1, tMax);

	float fracU = uv.x() - u0;
	float fracV = uv.y() - v0;

	const uint16_t* ptr = (const uint16_t*)mImage->getDataPointer();

	float d00 = float(ptr[u0 + mImage->s() * v0]);
	float d10 = float(ptr[u1 + mImage->s() * v0]);
	float d01 = float(ptr[u0 + mImage->s() * v1]);
	float d11 = float(ptr[u1 + mImage->s() * v1]);

	float d0 = skybolt::math::lerp(d00, d10, fracU);
	float d1 = skybolt::math::lerp(d01, d11, fracU);

	return getElevationForColorValue(mElevationRerange, skybolt::math::lerp(d0, d1, fracV));
}

} // namespace vis
} // namespace skybolt
