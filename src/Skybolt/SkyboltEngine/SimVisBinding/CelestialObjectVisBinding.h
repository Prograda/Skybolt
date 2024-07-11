/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltEngineFwd.h"
#include "SimVisBinding.h"

namespace skybolt {

typedef std::function<sim::LatLon(double)> LatLonProvider;

class CelestialObjectVisBinding : public SimVisBinding
{
public:
	CelestialObjectVisBinding(JulianDateProvider dateProvider, LatLonProvider eclipticDirectionProvider, const vis::RootNodePtr& visObject);
	void syncVis(const GeocentricToNedConverter& converter) override;

protected:
	JulianDateProvider mDateProvider;

private:
	LatLonProvider mEclipticDirectionProvider;
	vis::RootNodePtr mVisObject;
};

} // namespace skybolt