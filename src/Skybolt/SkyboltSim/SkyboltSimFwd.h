/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <memory>

namespace skybolt {
namespace sim {

class AttacherComponent;
struct AttachmentPoint;
class AttachmentPointsComponent;
class CameraController;
class CameraControllerComponent;
class CameraComponent;
struct CameraState;
class CollisionSystem;
class Component;
class ControlInputsComponent;
class DynamicBodyComponent;
class Entity;
struct EntityId;
struct LatLon;
struct LatLonAlt;
class MainRotorComponent;
class Motion;
class NameComponent;
class Node;
struct Orientation;
struct Particle;
class ParticleEmitter;
class ParticleSystem;
struct PlanetComponent;
struct Position;
class Positionable;
class PropellerComponent;
class SimStepper;
class System;
class World;

typedef std::shared_ptr<AttacherComponent> AttacherComponentPtr;
typedef std::shared_ptr<AttachmentPoint> AttachmentPointPtr;
typedef std::shared_ptr<AttachmentPointsComponent> AttachmentPointsComponentPtr;
typedef std::shared_ptr<CameraController> CameraControllerPtr;
typedef std::shared_ptr<CameraControllerComponent> CameraControllerComponentPtr;
typedef std::shared_ptr<CollisionSystem> CollisionSystemPtr;
typedef std::shared_ptr<Component> ComponentPtr;
typedef std::shared_ptr<Entity> EntityPtr;
typedef std::shared_ptr<MainRotorComponent> MainRotorComponentPtr;
typedef std::shared_ptr<NameComponent> NameComponentPtr;
typedef std::shared_ptr<Node> NodePtr;
typedef std::shared_ptr<Orientation> OrientationPtr;
typedef std::shared_ptr<ParticleEmitter> ParticleEmitterPtr;
typedef std::shared_ptr<ParticleSystem> ParticleSystemPtr;
typedef std::shared_ptr<PlanetComponent> PlanetComponentPtr;
typedef std::shared_ptr<Position> PositionPtr;
typedef std::shared_ptr<Positionable> PositionablePtr;
typedef std::shared_ptr<PropellerComponent> PropellerComponentPtr;
typedef std::shared_ptr<System> SystemPtr;

} // namespace sim
} // namespace skybolt