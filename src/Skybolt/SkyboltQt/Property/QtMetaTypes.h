/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "PropertyModel.h"
#include <SkyboltSim/Spatial/LatLon.h>

struct OptionalProperty
{
	QtPropertyPtr property;
	bool present = false;
};

Q_DECLARE_METATYPE(OptionalProperty)
Q_DECLARE_METATYPE(skybolt::sim::LatLon)