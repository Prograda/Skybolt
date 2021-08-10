/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <memory>

namespace skybolt {
namespace sim {

class AttachmentComponent;
struct AttachmentPoint;
class CameraController;
class CameraControllerSelector;
class CameraComponent;
class Component;
class DynamicBodyComponent;
class Entity;
struct LatLon;
class MainRotorComponent;
class NameComponent;
class NamedObjectRegistry;
class Node;
struct Orientation;
struct Particle;
class ParticleEmitter;
class ParticleSystem;
class ParentReferenceComponent;
struct Position;
class Positionable;
class PropellerComponent;
class SimStepper;
class System;
class TriangleMeshShapeData;
class World;

typedef std::shared_ptr<AttachmentComponent> AttachmentComponentPtr;
typedef std::shared_ptr<AttachmentPoint> AttachmentPointPtr;
typedef std::shared_ptr<CameraController> CameraControllerPtr;
typedef std::shared_ptr<CameraControllerSelector> CameraControllerSelectorPtr;
typedef std::shared_ptr<Component> ComponentPtr;
typedef std::shared_ptr<Entity> EntityPtr;
typedef std::shared_ptr<MainRotorComponent> MainRotorComponentPtr;
typedef std::shared_ptr<NameComponent> NameComponentPtr;
typedef std::shared_ptr<NamedObjectRegistry> NamedObjectRegistryPtr;
typedef std::shared_ptr<Node> NodePtr;
typedef std::shared_ptr<Orientation> OrientationPtr;
typedef std::shared_ptr<ParentReferenceComponent> ParentReferenceComponentPtr;
typedef std::shared_ptr<ParticleEmitter> ParticleEmitterPtr;
typedef std::shared_ptr<ParticleSystem> ParticleSystemPtr;
typedef std::shared_ptr<Position> PositionPtr;
typedef std::shared_ptr<Positionable> PositionablePtr;
typedef std::shared_ptr<PropellerComponent> PropellerComponentPtr;
typedef std::shared_ptr<System> SystemPtr;
typedef std::shared_ptr<TriangleMeshShapeData> TriangleMeshShapeDataPtr;
typedef std::shared_ptr<World> WorldPtr;

} // namespace sim
} // namespace skybolt