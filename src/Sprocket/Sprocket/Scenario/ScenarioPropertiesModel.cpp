/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "ScenarioPropertiesModel.h"
#include "Sprocket/Property/PropertyMetadata.h"
#include <SkyboltSim/Entity.h>
#include <SkyboltSim/Components/NameComponent.h>
#include <SkyboltSim/Physics/Astronomy.h>
#include <QDateTime>
#include <QTimeZone>

using namespace skybolt;

ScenarioPropertiesModel::ScenarioPropertiesModel(Scenario* scenario) :
	mScenario(scenario)
{
	assert(mScenario);

	{
		mDateTime = createQtProperty("startTime", QDateTime());
		mProperties.push_back(mDateTime);

		connect(mDateTime.get(), &QtProperty::valueChanged, [this]() {
			QDateTime dateTime = mDateTime->value.toDateTime();
			const QDate& date = dateTime.date();
			const QTime& time = dateTime.time();
			double hourF = double(time.hour()) + double(time.minute()) / 60.0 + double(time.second()) / (60.0*60.0);
			mScenario->startJulianDate = sim::calcJulianDate(date.year(), date.month(), date.day(), hourF);
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
		mTemporalMode = createQtProperty("temporalMode", 0);
		mTemporalMode->setProperty(PropertyMetadataNames::enumValueDisplayNames, QStringList({"Progressive", "Random Access"}));
		mProperties.push_back(mTemporalMode);

		connect(mTemporalMode.get(), &QtProperty::valueChanged, [this]() {
			mScenario->temporalMode.set(skybolt::TemporalMode(mTemporalMode->value.toInt()));
		});
	}

	update();
}

void ScenarioPropertiesModel::update()
{
	{
		QDate date = QDate::fromJulianDay((int)mScenario->startJulianDate);

		int h, m;
		double s;
		sim::julianDateToHms(mScenario->startJulianDate - (int)mScenario->startJulianDate, h, m, s);
		QTime time(h, m, (int)s);

		QDateTime dateTime(date, time, QTimeZone::utc());
		mDateTime->setValue(dateTime);
	}
	{
		mDuration->setValue(mScenario->timeSource.getRange().end);
	}
	{
		mTemporalMode->setValue(int(mScenario->temporalMode.get()));
	}
}
