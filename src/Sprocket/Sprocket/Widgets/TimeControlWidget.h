/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltEngine/TimeSource.h"
#include <QWidget>
#include <QAction>

#include <boost/signals2.hpp>

class TimeControlWidget : public QWidget
{
	Q_OBJECT
public:
	TimeControlWidget(QWidget* parent = 0);

	skybolt::TimeSource* getTimeSource() const { return mSource; }
	void setTimeSource(skybolt::TimeSource* source);

private:
	skybolt::TimeSource* mSource;

	QAction* mPlayAction;
	QAction* mForwardAction;
	QAction* mBackwardAction;

	std::vector<QMetaObject::Connection> mConnections;
	std::vector<boost::signals2::scoped_connection> mBoostConnections;
};
