/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <SkyboltEngine/SkyboltEngineFwd.h>
#include <SkyboltEngine/Scenario/Scenario.h>

#include "SkyboltQt/Property/PropertyModel.h"

class ScenarioPropertiesModel : public PropertiesModel
{
public:
	ScenarioPropertiesModel(skybolt::Scenario* scenario);

	void update() override;

private:
	skybolt::Scenario* mScenario;
	QtPropertyPtr mStartDateTime;
	QtPropertyPtr mDuration;
	QtPropertyPtr mTimelineMode;
};
