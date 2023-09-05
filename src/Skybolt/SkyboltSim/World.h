/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#pragma once

#include "SkyboltSim/Entity.h"
#include <SkyboltCommon/Event.h>
#include <SkyboltCommon/Listenable.h>

namespace skybolt {
namespace sim {

class WorldListener
{
public:
	virtual ~WorldListener() {}

	virtual void entityAdded(const sim::EntityPtr& entity) {}
	virtual void entityAboutToBeRemoved(const sim::EntityPtr& entity) {}
	virtual void entityRemoved(const sim::EntityPtr& entity) {}
};

class World : public EventEmitter, public skybolt::Listenable<WorldListener>
{
public:
	World();
	~World();

	Vector3 calcGravity(const Vector3& position, double mass) const;

	void addEntity(const EntityPtr& entity);
	void removeEntity(Entity* entity);
	void removeAllEntities();

	typedef std::vector<EntityPtr> Entities;
	inline const Entities &getEntities() const { return mEntities; }

	//! @return null if entity not found
	EntityPtr getEntityById(EntityId id) const;

	//! @return null if entity not found
	EntityPtr findObjectByName(const std::string& name) const;

private:
	Entities mEntities;
	std::map<EntityId, EntityPtr> mIdToEntityMap;
	std::map<std::string, EntityPtr> mNameToEntityMap;

	bool mDestructing = false;
};


} // namespace sim
} // namespace skybolt