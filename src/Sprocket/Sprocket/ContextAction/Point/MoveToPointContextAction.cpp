/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "MoveToPointContextAction.h"
#include <SkyboltSim/Spatial/Geocentric.h>
#include <SkyboltSim/Spatial/GreatCircle.h>
#include <SkyboltSim/Spatial/Orientation.h>

using namespace skybolt;
using namespace skybolt::sim;

bool MoveToPointContextAction::handles(const ActionContext& context) const
{
	return context.entity && getPosition(*context.entity).has_value() && context.point;
}

void MoveToPointContextAction::execute(ActionContext& context) const
{
	double currentAltitude = geocentricToLla(*getPosition(*context.entity), earthRadius()).alt;

	sim::LatLon newLatLon = geocentricToLatLon(*context.point);
	Vector3 newPosition = llaToGeocentric(sim::toLatLonAlt(newLatLon, currentAltitude), earthRadius());
	setPosition(*context.entity, newPosition);

	LtpNedOrientation ltpOrientation = toLtpNed(GeocentricOrientation(getOrientation(*context.entity).value_or(math::dquatIdentity())), newLatLon);
	Vector3 euler = math::eulerFromQuat(ltpOrientation.orientation);
	euler.x = 0;
	euler.y = 0;

	setOrientation(*context.entity, sim::toGeocentric(LtpNedOrientation(math::quatFromEuler(euler)), newLatLon).orientation);
}
