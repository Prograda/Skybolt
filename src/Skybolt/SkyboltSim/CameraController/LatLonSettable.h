/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltSim/Spatial/LatLon.h"

namespace skybolt {
namespace sim {

class LatLonSettable
{
	SKYBOLT_ENABLE_POLYMORPHIC_REFLECTION();
public:
	virtual ~LatLonSettable() = default;
	virtual const LatLon& getLatLon() const { return mLatLon; }
	virtual void setLatLon(const LatLon& latLon) { mLatLon = latLon; }

protected:
	LatLon mLatLon = LatLon(0,0);
};

SKYBOLT_REFLECT_INLINE(LatLonSettable)
{
	rttr::registration::class_<LatLonSettable>("LatLonSettable")
		.property("latLon", &LatLonSettable::getLatLon, &LatLonSettable::setLatLon);
}

} // namespace sim
} // namespace skybolt