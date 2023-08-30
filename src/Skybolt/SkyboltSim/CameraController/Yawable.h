/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <SkyboltCommon/Units.h>
#include <SkyboltSim/Reflection.h>

namespace skybolt {
namespace sim {

class Yawable
{
	SKYBOLT_ENABLE_POLYMORPHIC_REFLECTION();
public:
	virtual ~Yawable() = default;
	virtual double getYaw() const { return mYaw; }
	virtual void setYaw(double yaw) { mYaw = yaw; }

protected:
	double mYaw = 0;
};

SKYBOLT_REFLECT_INLINE(Yawable)
{
	rttr::registration::class_<Yawable>("Yawable")
		.property("yaw", &Yawable::getYaw, &Yawable::setYaw)
		(    
			rttr::metadata(PropertyMetadataType::Units, Units::Radians)
		);
}

} // namespace sim
} // namespace skybolt