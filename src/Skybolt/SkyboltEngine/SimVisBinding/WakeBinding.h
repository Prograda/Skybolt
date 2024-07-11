/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SimVisBinding.h"
#include <SkyboltVis/Renderable/Water/WaterMaterial.h>

namespace skybolt {

typedef std::vector<sim::PositionPtr> Positions;
typedef std::shared_ptr<Positions> PositionsPtr;

namespace sim
{

struct Wake
{
	Positions positions;
};
}

class WakeBinding : public SimVisBinding
{
public:
	WakeBinding(const sim::World* simWorld, const osg::ref_ptr<vis::WaterMaterial>& waterMaterial);

public:
	// SimVisBinding interface
	void syncVis(const GeocentricToNedConverter& converter) override;

private:
	const sim::World* mWorld;
	osg::ref_ptr<vis::WaterMaterial> mWaterMaterial;
};

} // namespace skybolt