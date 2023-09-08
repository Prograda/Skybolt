/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <functional>
#include <memory>

namespace skybolt {

class CameraInputSystem;
class ComponentFactory;
class EngineRoot;
struct EngineStats;
class EntityFactory;
class EntityStateSequenceController;
class EntityVisibilityFilterable;
class ForcesVisBinding;
class GeocentricToNedConverter;
class InputContext;
class InputDevice;
class InputPlatform;
class LogicalAxis;
class Plugin;
class PolylineVisBinding;
struct Scenario;
class SimVisBinding;
class StateSequenceController;
class StatsDisplaySystem;
class TimeSource;
class TriggerZone;
class Updatable;
class VisHud;
class VisNameLabels;
class VisObjectsComponent;

typedef std::shared_ptr<CameraInputSystem> CameraInputSystemPtr;
typedef std::shared_ptr<ComponentFactory> ComponentFactoryPtr;
typedef std::shared_ptr<LogicalAxis> LogicalAxisPtr;
typedef std::shared_ptr<InputDevice> InputDevicePtr;
typedef std::shared_ptr<InputPlatform> InputPlatformPtr;
typedef std::shared_ptr<Plugin> PluginPtr;
typedef std::shared_ptr<PolylineVisBinding> PolylineVisBindingPtr;
typedef std::shared_ptr<SimVisBinding> SimVisBindingPtr;
typedef std::shared_ptr<StateSequenceController> StateSequenceControllerPtr;
typedef std::shared_ptr<VisObjectsComponent> VisObjectsComponentPtr;

typedef std::function<double()> JulianDateProvider;

} // namespace skybolt