/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "OsgGeocentric.h"
#include <SkyboltCommon/Math/MathUtility.h>

namespace skybolt {
namespace vis {

osg::Vec3d llaToGeocentric(const osg::Vec2d& latLon, double altitude, double planetRadius)
{
	double cosLat = cos(latLon.x());
	return osg::Vec3d(cos(latLon.y()) * cosLat, sin(latLon.y()) * cosLat, sin(latLon.x())) * (planetRadius + altitude);
}

void geocentricToLla(const osg::Vec3d& pos, osg::Vec2d& latLon, double& altitude, double planetRadius)
{
	altitude = pos.length() - planetRadius;
	latLon.x() = atan(pos.z() / osg::Vec2d(pos.x(), pos.y()).length());
	latLon.y() = atan2(pos.y(), pos.x());
}

osg::Quat latLonToGeocentricLtpOrientation(const osg::Vec2d& latLon)
{	
	return osg::Quat(latLon.x() + skybolt::math::halfPiD(), osg::Vec3f(0, -1, 0)) * osg::Quat(latLon.y(), osg::Vec3f(0, 0, 1));
}

} // namespace vis
} // namespace skybolt