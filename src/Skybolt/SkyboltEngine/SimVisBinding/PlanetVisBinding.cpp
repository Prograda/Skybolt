/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "PlanetVisBinding.h"
#include <SkyboltVis/Renderable/Planet/Planet.h>

namespace skybolt {

PlanetVisBinding::PlanetVisBinding(JulianDateProvider dateProvider, const sim::Entity* entity, const vis::PlanetPtr& visObject) :
	SimpleSimVisBinding(entity, visObject),
	mDateProvider(dateProvider)
{}

void PlanetVisBinding::syncVis(const GeocentricToNedConverter& converter)
{
	SimpleSimVisBinding::syncVis(converter);

	static_cast<vis::Planet*>(mVisObjects.front().object.get())->setJulianDate(mDateProvider());
}

} // namespace skybolt