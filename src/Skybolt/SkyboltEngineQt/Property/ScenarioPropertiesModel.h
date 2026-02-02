#pragma once

#include <SkyboltEngine/SkyboltEngineFwd.h>
#include <SkyboltEngine/Scenario/Scenario.h>
#include <SkyboltWidgets/Property/QtProperty.h>

class ScenarioPropertiesModel : public skybolt::PropertiesModel
{
public:
	ScenarioPropertiesModel(skybolt::Scenario* scenario);

	void update();

private:
	skybolt::Scenario* mScenario;
	skybolt::QtPropertyPtr mStartDateTime;
	skybolt::QtPropertyPtr mDuration;
	skybolt::QtPropertyPtr mTimelineMode;
};
