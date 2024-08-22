/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <SkyboltCommon/ObservableValue.h>
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

	skybolt::ObservableValueD* getRequestedTimeRateSource() const { return mRequestedTimeRateSource; }
	void setRequestedTimeRateSource(skybolt::ObservableValueD* source);

	skybolt::ObservableValueD* getActualTimeRateSource() const { return mActualTimeRateSource; }
	void setActualTimeRateSource(skybolt::ObservableValueD* source);

	void setTimelineMode(skybolt::TimelineMode timelineMode);

private:
	void updateTimeRateLabel();

private:
	skybolt::TimeSource* mSource = nullptr;
	skybolt::ObservableValueD* mRequestedTimeRateSource = nullptr;
	skybolt::ObservableValueD* mActualTimeRateSource = nullptr;
	skybolt::TimelineMode mTimelineMode = skybolt::TimelineMode::Free;

	QAction* mPlayAction;
	QAction* mForwardAction;
	QAction* mBackwardAction;
	QLabel* mTimeRateLabel;

	std::unique_ptr<class TimeRateDialog> mTimeRateDialog;

	std::vector<QMetaObject::Connection> mQtTimeSourceConnections;
	boost::signals2::scoped_connection mBoostTimeSourceConnection;
	boost::signals2::scoped_connection mBoostRequestedTimeRateSourceConnection;
	boost::signals2::scoped_connection mBoostActualTimeRateSourceConnection;
};
