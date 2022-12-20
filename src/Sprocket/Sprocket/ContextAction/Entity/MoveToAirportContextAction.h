/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "Sprocket/ContextAction/Entity/EntityContextAction.h"
#include <SkyboltSim/Entity.h>
#include <SkyboltVis/Renderable/Planet/Features/PlanetFeaturesSource.h>
#include <string>

typedef std::map<std::string, skybolt::mapfeatures::AirportPtr> AirportsMap;

class MoveToAirportContextAction : public EntityContextAction
{
public:
	MoveToAirportContextAction(const AirportsMap& airports) : mAirports(airports) {}

	std::string getName() const override { return "Move to Airport"; }

	virtual bool handles(const skybolt::sim::Entity& entity) const override;

	virtual void execute(skybolt::sim::Entity& entity) const override;

private:
	const AirportsMap mAirports;
};
