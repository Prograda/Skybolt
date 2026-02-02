#include "ScenarioPropertiesModel.h"
#include "Util/QtDateTimeUtil.h"

#include <SkyboltWidgets/Property/QtPropertyMetadata.h>
#include <SkyboltSim/Entity.h>
#include <SkyboltSim/Components/NameComponent.h>
#include <QDateTime>

using namespace skybolt;

ScenarioPropertiesModel::ScenarioPropertiesModel(Scenario* scenario) :
	mScenario(scenario)
{
	assert(mScenario);

	{
		mStartDateTime = createQtProperty("startTime", QDateTime());
		mProperties[PropertiesModel::getDefaultSectionName()].push_back(mStartDateTime);

		connect(mStartDateTime.get(), &QtProperty::valueChanged, [this]() {
			QDateTime dateTime = mStartDateTime->value().toDateTime();
			mScenario->startJulianDate = qdateTimeToJulianDate(dateTime);
		});
	}
	{
		mDuration = createQtProperty("duration", 0.0);
		mProperties[PropertiesModel::getDefaultSectionName()].push_back(mDuration);

		connect(mDuration.get(), &QtProperty::valueChanged, [this]() {
			TimeRange range = mScenario->timeSource.getRange();
			range.end = mDuration->value().toDouble();
			mScenario->timeSource.setRange(range);
		});
	}
	{
		mTimelineMode = createQtProperty("timelineMode", 0);
		mTimelineMode->setProperty(QtPropertyMetadataKeys::optionNames, QStringList({"Live", "Free"}));
		mProperties[PropertiesModel::getDefaultSectionName()].push_back(mTimelineMode);

		connect(mTimelineMode.get(), &QtProperty::valueChanged, [this]() {
			mScenario->timelineMode.set(skybolt::TimelineMode(mTimelineMode->value().toInt()));
		});
	}

	update();
}

void ScenarioPropertiesModel::update()
{
	mStartDateTime->setValue(julianDateToQDateTime(mScenario->startJulianDate));
	mDuration->setValue(mScenario->timeSource.getRange().end);
	mTimelineMode->setValue(int(mScenario->timelineMode.get()));
}
