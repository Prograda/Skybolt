/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SimVisBinding.h"
#include <SkyboltVis/Renderable/Polyline.h>

namespace skybolt {

typedef std::vector<sim::PositionPtr> Positions;
typedef std::shared_ptr<Positions> PositionsPtr;

class PolylineVisBinding : public SimVisBinding
{
public:
	PolylineVisBinding(const vis::PolylinePtr& polyline);

	void setPoints(const PositionsPtr& points);

public:
	// SimVisBinding interface
	void syncVis(const GeocentricToNedConverter& converter) override;

private:
	vis::PolylinePtr mPolyline;
	PositionsPtr mPoints;
};

} // namespace skybolt