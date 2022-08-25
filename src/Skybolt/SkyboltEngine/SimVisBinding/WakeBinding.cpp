/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "WakeBinding.h"
#include "GeocentricToNedConverter.h"
#include <SkyboltSim/Entity.h>
#include <SkyboltSim/World.h>
#include <SkyboltSim/Components/DynamicBodyComponent.h>
#include <SkyboltSim/Components/ShipWakeComponent.h>
#include <SkyboltSim/Spatial/Geocentric.h>
#include <SkyboltSim/Spatial/GreatCircle.h>
#include <SkyboltSim/Spatial/Position.h>
#include <SkyboltVis/Renderable/Water/WaterStateSet.h>
#include <assert.h>

namespace skybolt {

WakeBinding::WakeBinding(const sim::World* simWorld, const osg::ref_ptr<vis::WaterMaterial>& waterMaterial) :
	mWorld(simWorld),
	mWaterMaterial(waterMaterial)
{
	assert(mWorld);
	assert(mWaterMaterial);
}

void WakeBinding::syncVis(const GeocentricToNedConverter& converter)
{
	std::vector<vis::Wake> wakes;

	for (const sim::EntityPtr& entity : mWorld->getEntities())
	{
		if (auto* component = entity->getFirstComponent<sim::ShipWakeComponent>().get())
		{
			auto position = getPosition(*entity);
			if (position)
			{
				auto platformPosition = std::make_shared<sim::GeocentricPosition>(*position);
				auto wakeEndPosition = sim::toLatLonAlt(*platformPosition);

				sim::Vector3 dir = *getOrientation(*entity) * sim::Vector3(1, 0, 0);

				osg::Vec3d wakeDir = converter.convertLocalPosition(dir);
				wakeDir.z() = 0;
				wakeDir.normalize();

				vis::Wake wake;
				wake.points.resize(2);

				osg::Vec3f startPoint = converter.convertPosition(*position);
				if (component->type == sim::ShipWakeComponent::Type::SHIP_WAKE)
				{
					wake.points[0] = startPoint;
					wake.points[0].z() = component->startRadius;
					wake.points[1] = startPoint - wakeDir * component->length;
					wake.points[1].z() = component->endRadius;
				}
				else if(component->type == sim::ShipWakeComponent::Type::ROTOR_WASH)
				{
					wake.points[0] = startPoint;
					wake.points[0].z() = -40; // negative indicates rotor wash
					wake.points[1] = startPoint - wakeDir * 0.1;
					wake.points[1].z() = 40;
				}
				else
				{
					assert(!"Not implemented");
				}

				wakes.push_back(wake);
			}
		}
	}

	mWaterMaterial->setWakes(wakes);
}

} // namespace skybolt