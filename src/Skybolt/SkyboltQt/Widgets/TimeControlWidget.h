/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <SkyboltEngine/TimeSource.h>
#include <SkyboltEngine/Scenario/Scenario.h>
#include <QWidget>
#include <QAction>

#include <boost/signals2.hpp>

class QDialog;
class QLabel;

class TimeControlWidget : public QWidget
{
	Q_OBJECT
public:
	TimeControlWidget(QWidget* parent = 0);
	~TimeControlWidget();

	skybolt::TimeSource* getTimeSource() const { return mSource; }
	void setTimeSource(skybolt::TimeSource* source);

	void setTimelineMode(skybolt::TimelineMode timelineMode);

	double getRequestedTimeRate() const;
	void setRequestedTimeRate(double rate);

	void setActualTimeRate(double rateTimesRealtime);

private:
	void updateTimeRateLabel();

private:
	skybolt::TimeSource* mSource;
	skybolt::TimelineMode mTimelineMode = skybolt::TimelineMode::Free;

	QAction* mPlayAction;
	QAction* mForwardAction;
	QAction* mBackwardAction;
	QLabel* mTimeRateLabel;

	std::unique_ptr<class TimeRateDialog> mTimeRateDialog;
	double mRequestedTimeRate = 1;
	double mActualTimeRate = 1;

	std::vector<QMetaObject::Connection> mConnections;
	std::vector<boost::signals2::scoped_connection> mBoostConnections;
};
