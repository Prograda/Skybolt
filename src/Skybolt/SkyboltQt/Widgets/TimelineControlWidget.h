/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <SkyboltEngine/SkyboltEngineFwd.h>
#include <SkyboltEngine/Scenario/Scenario.h>

#include <QWidget>

class TimeControlWidget;

struct TimelineControlWidgetConfig
{
	skybolt::TimeSource* timeSource;
	skybolt::ObservableValue<skybolt::TimelineMode>* timelineMode;
	skybolt::ObservableValueD* requestedTimeRate;
	skybolt::ObservableValueD* actualTimeRate;
	QWidget* parent = nullptr;
};

class TimelineControlWidget : public QWidget
{
public:
	TimelineControlWidget(const TimelineControlWidgetConfig& config);

	TimeControlWidget* getTimeControlWidget() const { return mTimeControlWidget; }

private:
	TimeControlWidget* mTimeControlWidget;
};
