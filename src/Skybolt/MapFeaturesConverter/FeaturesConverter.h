/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <SkyboltSim/PlanetAltitudeProvider.h>
#include <SkyboltVis/Renderable/Planet/Features/PlanetFeaturesSource.h>

namespace skybolt {
namespace mapfeatures {

struct ReadPbfResult
{
	std::vector<FeaturePtr> features; //!< All the features
	std::map<std::string, AirportPtr> airports; //!< Map of names to airport features
};
ReadPbfResult readPbf(const std::string& filename, const sim::PlanetAltitudeProvider& provider);

} // namespace mapfeatures
} // namespace skybolt