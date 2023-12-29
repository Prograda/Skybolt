#pragma once

#include <memory>

namespace skybolt {

class Event;
class EventEmitter;
class Random;

using EventEmitterPtr = std::shared_ptr<EventEmitter>;
using RandomPtr = std::shared_ptr<Random>;

} // namespace skybolt