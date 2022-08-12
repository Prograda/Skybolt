/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "LlaToNedConverter.h"
#include "SkyboltSim/Spatial/GreatCircle.h"

namespace skybolt {
namespace vis {

inline osg::Vec2f toVec2f(const glm::dvec2& v)
{
	return osg::Vec2f(v.x, v.y);
}

inline glm::dvec2 toDvec2(const osg::Vec2f& v)
{
	return glm::dvec2(v.x(), v.y());
}

osg::Vec2f LlaToNedConverter::latLonToCartesianNe(const sim::LatLon& position) const
{
	return toVec2f(sim::latLonToCartesianNe(mOrigin, position));
}

osg::Vec3f LlaToNedConverter::latLonAltToCartesianNed(const sim::LatLonAlt& position) const
{
	glm::dvec2 ne = sim::latLonToCartesianNe(mOrigin, toLatLon(position));
	double d = -position.alt;

	if (mPlanetRadiusForSurfaceDrop)
	{
		d += calcPlanetSurfaceDrop(glm::length(ne));
	}

	return osg::Vec3f(ne.x, ne.y, d);
}

sim::LatLon LlaToNedConverter::cartesianNeToLatLon(const osg::Vec2f& position) const
{
	return sim::cartesianNeToLatLon(mOrigin, toDvec2(position));
}

float LlaToNedConverter::calcPlanetSurfaceDrop(float distance) const
{
	return distance * distance / (2 * *mPlanetRadiusForSurfaceDrop);
}

} // namespace vis
} // namespace skybolt
