/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "PolylineVisBinding.h"
#include "GeocentricToNedConverter.h"
#include <SkyboltSim/Spatial/Position.h>
#include <SkyboltSim/Spatial/Geocentric.h>
#include <assert.h>

namespace skybolt {

PolylineVisBinding::PolylineVisBinding(const vis::PolylinePtr& polyline) :
	mPolyline(polyline)
{
	assert(polyline);
}

void PolylineVisBinding::setPoints(const PositionsPtr& points)
{
	mPoints = points;
}

void PolylineVisBinding::syncVis(const GeocentricToNedConverter& converter)
{
	if (mPoints)
	{
		osg::ref_ptr<osg::Vec3Array> points(new osg::Vec3Array(mPoints->size()));
		int i = 0;
		for (const sim::PositionPtr& position : *mPoints)
		{
			sim::GeocentricPosition geocentricPosition = sim::toGeocentric(*position);
			(*points)[i] = converter.convertPosition(geocentricPosition.position);
			++i;
		}
		mPolyline->setPoints(points);
	}
	else
	{
		mPolyline->setPoints(nullptr);
	}
}

} // namespace skybolt