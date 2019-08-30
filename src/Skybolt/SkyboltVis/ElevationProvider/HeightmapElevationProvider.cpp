/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "HeightmapElevationProvider.h"
#include <SkyboltCommon/Math/MathUtility.h>

namespace skybolt {
namespace vis {

HeightmapElevationProvider::HeightmapElevationProvider(const osg::ref_ptr<osg::Image>& image, const Box2f& bounds) :
	image(image), offset(bounds.minimum.y(), bounds.minimum.x())
{
	scale = osg::Vec2f(image->s() / (bounds.maximum.y() - bounds.minimum.y()),
		image->t() / (bounds.maximum.x() - bounds.minimum.x()));
}

float HeightmapElevationProvider::get(float x, float y) const
{
	osg::Vec3f uv = osg::Vec3f((y - offset.x()) * scale.x(),
		(x - offset.y()) * scale.y(), 0.0f);

	int sMax = image->s() - 1;
	int tMax = image->t() - 1;

	uv.x() = skybolt::math::clamp(uv.x(), 0.0f, float(sMax));
	uv.y() = skybolt::math::clamp(uv.y(), 0.0f, float(tMax));

	int u0 = (int)uv.x();
	int u1 = std::min(u0 + 1, sMax);
	int v0 = (int)uv.y();
	int v1 = std::min(v0 + 1, tMax);

	float fracU = uv.x() - u0;
	float fracV = uv.y() - v0;

	const uint16_t* ptr = (const uint16_t*)image->getDataPointer();

	float d00 = float(ptr[u0 + image->s() * v0]);
	float d10 = float(ptr[u1 + image->s() * v0]);
	float d01 = float(ptr[u0 + image->s() * v1]);
	float d11 = float(ptr[u1 + image->s() * v1]);

	float d0 = skybolt::math::lerp(d00, d10, fracU);
	float d1 = skybolt::math::lerp(d01, d11, fracU);

	return heightmapValueToFloat(skybolt::math::lerp(d0, d1, fracV));
}

} // namespace vis
} // namespace skybolt
