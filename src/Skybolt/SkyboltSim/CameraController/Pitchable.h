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

class Pitchable
{
	SKYBOLT_ENABLE_POLYMORPHIC_REFLECTION();
public:
	~Pitchable() = default;
	virtual double getPitch() const { return mPitch; }
	virtual void setPitch(double pitch) { mPitch = pitch; }

protected:
	double mPitch = 0;
};

SKYBOLT_REFLECT_INLINE(Pitchable)
{
	rttr::registration::class_<Pitchable>("Pitchable")
		.property("pitch", &Pitchable::getPitch, &Pitchable::setPitch)
		(    
			rttr::metadata(PropertyMetadataType::Units, Units::Radians)
		);
}

} // namespace sim
} // namespace skybolt