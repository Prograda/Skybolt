/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltSim/SimMath.h"
#include "SkyboltSim/Spatial/LatLon.h"
#include "SkyboltSim/Spatial/LatLonAlt.h"

namespace skybolt {
namespace sim {

//! Convert Lat-Long-Altitude to Earth-Centred-Earth-Fixed (ECEF) coordinates.
//! Right handed coordinate system where +X is through 0 latitude and 0 longitude, +ve Z is through the north pole.
Vector3 llaToGeocentric(const LatLonAlt& lla, double planetRadius);

LatLonAlt geocentricToLla(const Vector3& pos, double planetRadius);

LatLon geocentricToLatLon(const Vector3& pos);

//! Returns the orientation of the north-east-down local-tangent-plane relative to geocentric axes
Quaternion latLonToGeocentricLtpOrientation(const LatLon& latLon);

//! Returns the north-east-down local-tangent-plane orientation for a geocentric point
Matrix3 geocentricToLtpOrientation(const Vector3& pos);

} // namespace skybolt
} // namespace sim