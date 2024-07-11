/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <SkyboltSim/Chrono.h>
#include <functional>

class QObject;
class QTimer;

QTimer* createAndStartIntervalTimer(int intervalMilliseconds, QObject* parent, std::function<void()> updateAction);

QTimer* createAndStartIntervalDtTimer(int intervalMilliseconds, QObject* parent, std::function<void(skybolt::sim::SecondsD dt)> updateAction);