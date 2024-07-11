/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "ScenarioPropertiesModel.h"
#include "SkyboltQt/Property/QtPropertyMetadata.h"
#include "SkyboltQt/QtUtil/QtDateTimeUtil.h"
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
		mProperties.push_back(mStartDateTime);

		connect(mStartDateTime.get(), &QtProperty::valueChanged, [this]() {
			QDateTime dateTime = mStartDateTime->value.toDateTime();
			mScenario->startJulianDate = qdateTimeToJulianDate(dateTime);
		});
	}
	{
		mDuration = createQtProperty("duration", 0.0);
		mProperties.push_back(mDuration);

		connect(mDuration.get(), &QtProperty::valueChanged, [this]() {
			TimeRange range = mScenario->timeSource.getRange();
			range.end = mDuration->value.toDouble();
			mScenario->timeSource.setRange(range);
		});
	}
	{
		mTimelineMode = createQtProperty("timelineMode", 0);
		mTimelineMode->setProperty(QtPropertyMetadataNames::optionNames, QStringList({"Live", "Free"}));
		mProperties.push_back(mTimelineMode);

		connect(mTimelineMode.get(), &QtProperty::valueChanged, [this]() {
			mScenario->timelineMode.set(skybolt::TimelineMode(mTimelineMode->value.toInt()));
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
