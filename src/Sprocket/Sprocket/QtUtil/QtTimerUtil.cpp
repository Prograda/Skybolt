/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "QtTimerUtil.h"
#include <QTimer>

#include <cxxtimer/cxxtimer.hpp>

QTimer* createAndStartIntervalTimer(int intervalMilliseconds, QObject* parent, std::function<void()> updateAction)
{
	auto timer = new QTimer(parent);
	timer->setInterval(intervalMilliseconds);

	QObject::connect(timer, &QTimer::timeout, updateAction);
	
	timer->start();
	return timer;
}

QTimer* createAndStartIntervalDtTimer(int intervalMilliseconds, QObject* parent, std::function<void(skybolt::sim::SecondsD dt)> updateAction)
{
	return createAndStartIntervalTimer(intervalMilliseconds, parent, [updateAction = std::move(updateAction), timeSinceLastUpdate = cxxtimer::Timer()] () mutable {
		double elapsed = double(timeSinceLastUpdate.count<std::chrono::milliseconds>()) / 1000.0;

		timeSinceLastUpdate.reset();
		timeSinceLastUpdate.start();

		updateAction(elapsed);
	});
}
