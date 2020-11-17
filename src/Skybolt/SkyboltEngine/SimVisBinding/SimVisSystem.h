/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltEngine/SkyboltEngineFwd.h"
#include <SkyboltSim/SkyboltSimFwd.h>
#include <SkyboltSim/System/System.h>

namespace skybolt {

//! Synchronizes the visualization state with the simulation state each update
class SimVisSystem : public sim::System
{
public:
	SimVisSystem(EngineRoot* engineRoot, const sim::EntityPtr& simCamera);
	~SimVisSystem();

	void updatePostDynamics(const System::StepArgs& args) override;

private:
	EngineRoot* mEngineRoot;
	sim::EntityPtr mSimCamera;
	std::unique_ptr<GeocentricToNedConverter> mCoordinateConverter;
};

} // namespace skybolt