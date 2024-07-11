/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <memory>

namespace skybolt {

class Event;
class EventEmitter;
class Random;

using EventEmitterPtr = std::shared_ptr<EventEmitter>;
using RandomPtr = std::shared_ptr<Random>;

} // namespace skybolt