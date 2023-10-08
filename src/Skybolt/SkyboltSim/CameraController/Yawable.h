/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltSim/PropertyMetadata.h"
#include <SkyboltCommon/Units.h>
#include <SkyboltReflection/Reflection.h>

namespace skybolt {
namespace sim {

class Yawable
{
public:
	virtual ~Yawable() = default;
	virtual double getYaw() const { return mYaw; }
	virtual void setYaw(double yaw) { mYaw = yaw; }

protected:
	double mYaw = 0;
};

SKYBOLT_REFLECT_BEGIN(Yawable)
{
	registry.type<Yawable>("Yawable")
		.property("yaw", &Yawable::getYaw, &Yawable::setYaw, {{PropertyMetadataNames::units, Units::Radians}});
}
SKYBOLT_REFLECT_END

} // namespace sim
} // namespace skybolt