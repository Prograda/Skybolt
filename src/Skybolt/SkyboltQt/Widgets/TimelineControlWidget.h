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

class TimelineControlWidget : public QWidget
{
public:
	TimelineControlWidget(skybolt::TimeSource* timeSource, skybolt::ObservableValue<skybolt::TimelineMode>* timelineMode, QWidget* parent = nullptr);

	TimeControlWidget* getTimeControlWidget() const { return mTimeControlWidget; }

private:
	TimeControlWidget* mTimeControlWidget;
};