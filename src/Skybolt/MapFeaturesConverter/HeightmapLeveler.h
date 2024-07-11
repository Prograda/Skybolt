/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <SkyboltVis/Renderable/Planet/Features/PlanetFeaturesSource.h>

#include <osg/Image>

namespace skybolt {
namespace mapfeatures {

void levelHeightmapsUnderFeatures(const std::string& heightmapSourceDirectory, const std::string& heightmapDestinationDirectory, const std::vector<Feature*>& features, double borderMeters);

double getAltitudeAtPosition(const std::string& heightmapSourceDirectory, const sim::LatLon& position);

} // namespace mapfeatures
} // namespace skybolt