/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "TimelineControlWidget.h"

#include "Widgets/TimeControlWidget.h"
#include "Widgets/TimelineWidget.h"

#include <SkyboltEngine/TimeSource.h>

#include <QBoxLayout>

using namespace skybolt;

TimelineControlWidget::TimelineControlWidget(TimeSource* timeSource, ObservableValue<TimelineMode>* timelineMode, QWidget* parent) :
	QWidget(parent)
{
	assert(timeSource);

	auto layout = new QVBoxLayout(this);
	setLayout(layout);

	TimelineWidget* timeline = new TimelineWidget;
	layout->addWidget(timeline);
	mTimeControlWidget = new TimeControlWidget;
	layout->addWidget(mTimeControlWidget);
	layout->addStretch();

	timeline->setTimeSource(timeSource);
	
	timeline->setTimelineMode(timelineMode->get());
	mTimeControlWidget->setTimelineMode(timelineMode->get());
	timelineMode->valueChanged.connect([timeline, this] (const TimelineMode& oldValue, const TimelineMode& newValue) {
		timeline->setTimelineMode(newValue);
		mTimeControlWidget->setTimelineMode(newValue);
	});

	timeSource->timeChanged.connect([=](double time)
	{
		timeline->setBufferedRange(TimeRange(0, time));
	});

	mTimeControlWidget->setTimeSource(timeSource);
}