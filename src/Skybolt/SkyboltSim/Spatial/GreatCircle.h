/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "LatLon.h"
#include <glm/glm.hpp>

namespace skybolt {
namespace sim {

inline double earthRadius() {return 6371000.0;}

double calcDistance(const sim::LatLon& p1, const sim::LatLon& p2);

//! @return bearing from p1 to p2. Result is clockwise from north in radians.
double calcBearing(const sim::LatLon& p1, const sim::LatLon& p2);

glm::dvec2 latLonToCartesianNe(const sim::LatLon& origin, const sim::LatLon& position);

sim::LatLon cartesianNeToLatLon(const sim::LatLon& origin, const glm::dvec2& position);

sim::LatLon moveDistanceAndBearing(const sim::LatLon& origin, double distance, double bearing);

} // namespace skybolt
} // namespace sim