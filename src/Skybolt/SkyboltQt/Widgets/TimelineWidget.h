/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <SkyboltEngine/TimeSource.h>
#include <SkyboltEngine/Scenario/Scenario.h>
#include <QWidget>
#include <QLabel>
#include <QSlider>

#include <boost/signals2.hpp>

class TimelineWidget : public QWidget
{
	Q_OBJECT
public:
	TimelineWidget(QWidget* parent = 0);

	skybolt::TimeSource* getTimeSource() const { return mSource; }
	void setTimeSource(skybolt::TimeSource* source);
	void setTimelineMode(skybolt::TimelineMode timelineMode);
	void setBufferedRange(const skybolt::TimeRange& range);

private:
	static QString toTimeText(double time);
	int toSliderValue(double time) const;

private slots:
	void timeChanged(double time);
	void rangeChanged(const skybolt::TimeRange& range);

private:
	skybolt::TimeSource* mSource;
	skybolt::TimelineMode mTimelineMode = skybolt::TimelineMode::Free;

	QLabel* mTime;
	QLabel* mDuration;
	class TimelineSlider* mSlider;
	skybolt::TimeRange mBufferedRange;

	std::vector<QMetaObject::Connection> mConnections;
	std::vector<boost::signals2::scoped_connection> mBoostConnections;
};
