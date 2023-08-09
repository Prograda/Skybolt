/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "QtLayoutUtil.h"
#include <QTimer>

QTimer* createAndStartIntervalTimer(int intervalMilliseconds, QObject* parent, std::function<void()> callback)
{
	auto timer = new QTimer(parent);
	timer->setInterval(intervalMilliseconds);

	QObject::connect(timer, &QTimer::timeout, callback);
	
	timer->start();
	return timer;
}