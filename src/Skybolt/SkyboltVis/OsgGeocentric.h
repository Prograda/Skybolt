/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltSim/SimMath.h"
#include "SkyboltSim/Spatial/LatLon.h"
#include "SkyboltSim/Spatial/LatLonAlt.h"
#include <osg/Quat>
#include <osg/Vec2d>
#include <osg/Vec3d>

namespace skybolt {
namespace vis {

//! Convert Lat-Long-Altitude to Earth-Centred-Earth-Fixed (ECEF) coordinates.
//! Right handed coordinate system where +X is through 0 latitude and 0 longitude, +ve Z is through the north pole.
osg::Vec3d llaToGeocentric(const osg::Vec2d& latLon, double altitude, double planetRadius);

void geocentricToLla(const osg::Vec3d& pos, osg::Vec2d& latLon, double& altitude, double planetRadius);

inline void geocentricToLla(const osg::Vec3d& pos, sim::LatLonAlt& lla, double planetRadius)
{
	geocentricToLla(pos, (osg::Vec2d&)lla, lla.alt, planetRadius);
}

//! Returns the orientation of the north-east-down local-tangent-plane relative to geocentric axes
osg::Quat latLonToGeocentricLtpOrientation(const osg::Vec2d& latLon);

} // namespace vis
} // namespace skybolt