/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "ScenarioObject.h"
#include <SkyboltSim/EntityId.h>
#include <SkyboltSim/SkyboltSimFwd.h>
#include <SkyboltEngine/SkyboltEngineFwd.h>

class EntityObjectRegistry;

class EntityObject : public ScenarioObjectT<skybolt::sim::EntityId>
{
public:
	EntityObject(EntityObjectRegistry* registry, skybolt::sim::World* world, const skybolt::sim::Entity& entity);
	
	const skybolt::ScenarioObjectPath& getDirectory() const override;
	void setDirectory(const skybolt::ScenarioObjectPath& path) override;

	std::optional<skybolt::sim::Vector3> getWorldPosition() const override;
	void setWorldPosition(const skybolt::sim::Vector3& position) override;

	std::optional<skybolt::sim::Vector3> intersectRay(const skybolt::sim::Vector3& origin, const skybolt::sim::Vector3& dir, const glm::dmat4& viewProjTransform) const override;

private:
	EntityObjectRegistry* mRegistry;
	skybolt::sim::World* mWorld;
};

using EntityObjectFactory = std::function<EntityObjectPtr(EntityObjectRegistry* registry, skybolt::sim::World* world, const skybolt::sim::Entity& entity)>;
EntityObjectFactory createDefaultEntityObjectFactory();
ScenarioObjectTypePtr createEntityObjectType(skybolt::sim::World* world, skybolt::EntityFactory* entityFactory, EntityObjectFactory entityObjectFactory = createDefaultEntityObjectFactory());