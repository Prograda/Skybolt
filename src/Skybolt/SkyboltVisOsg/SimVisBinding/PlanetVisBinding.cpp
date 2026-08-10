/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "PlanetVisBinding.h"
#include <SkyboltSim/Entity.h>
#include <SkyboltSim/Components/CloudComponent.h>
#include <SkyboltSim/Components/OceanComponent.h>
#include <SkyboltVisOsg/Renderable/Planet/Planet.h>
#include <SkyboltVisOsg/Renderable/Water/WaterMaterial.h>

namespace skybolt {

PlanetVisBinding::PlanetVisBinding(JulianDateProvider dateProvider, const sim::Entity* entity, const vis::PlanetPtr& visObject) :
	SimpleSimVisBinding(entity, visObject),
	mDateProvider(dateProvider)
{}

void PlanetVisBinding::syncVis(const GeocentricToNedConverter& converter)
{
	SimpleSimVisBinding::syncVis(converter);

	auto visPlanet = static_cast<vis::Planet*>(mVisObjects.front().object.get());
	visPlanet->setJulianDate(mDateProvider());

	if (auto ocean = mEntity->getFirstComponent<sim::OceanComponent>(); ocean)
	{
		if (const auto& material = visPlanet->getWaterMaterial(); material)
		{
			material->setWaveHeight(ocean->waveHeight);
			material->setWindVelocityHeading(ocean->windVelocityHeading);
		}
	}

	if (auto cloud = mEntity->getFirstComponent<sim::CloudComponent>(); cloud)
	{
		visPlanet->setCloudsVisible(!cloud->layers.empty());
		visPlanet->setCloudCoverageFraction(float(cloud->layers.size() ? cloud->layers.front().coverageFraction : 0.0));
	}
}

} // namespace skybolt