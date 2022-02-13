/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltSimFwd.h"
#include "Component.h"
#include "SimMath.h"
#include <SkyboltCommon/Exception.h>
#include <SkyboltCommon/Listenable.h>
#include <SkyboltCommon/TypedItemContainer.h>

#include <optional>

namespace skybolt {
namespace sim {

class EntityListener
{
public:
	virtual ~EntityListener() {}
	virtual void onComponentAdded(Entity* entity, Component* component) {}
	virtual void onComponentRemove(Entity* entity, Component* component) {}
	virtual void onDestroy(Entity* entity) {}
};

class Entity : public skybolt::Listenable<EntityListener>
{
public:
	Entity();
	virtual ~Entity();

	void updatePreDynamics(TimeReal dt, TimeReal dtWallClock);
	void updatePreDynamicsSubstep(TimeReal dtSubstep);
	void updatePostDynamicsSubstep(TimeReal dtSubstep);
	void updatePostDynamics(TimeReal dt, TimeReal dtWallClock);
	void updateAttachments(TimeReal dt, TimeReal dtWallClock);

	void setDynamicsEnabled(bool enabled);
	bool isDynamicsEnabled() const { return mDynamicsEnabled; }

	void addComponent(const ComponentPtr& c);
	void removeComponent(const ComponentPtr& c);

	template <class DerivedT>
	std::vector<std::shared_ptr<DerivedT>> getComponentsOfType() const
	{
		return mComponents.getItemsOfType<DerivedT>();
	}

	std::vector<ComponentPtr> getComponents() const
	{
		return mComponents.getAllItems();
	}

	template <class DerivedT>
	std::shared_ptr<DerivedT> getFirstComponent() const
	{
		return mComponents.getFirstItemOfType<DerivedT>();
	}

	template <class DerivedT>
	std::shared_ptr<DerivedT> getFirstComponentRequired() const
	{
		auto component = getFirstComponent<DerivedT>();
		if (!component)
		{
			throw Exception("Could not find component: " + std::string(typeid(DerivedT).name()));
		}
		return component;
	}

private:
	TypedItemContainer<Component> mComponents;
	bool mDynamicsEnabled = true;
};

std::optional<Vector3> getPosition(const Entity& entity);
std::optional<Quaternion> getOrientation(const Entity& entity);
std::optional<Vector3> getVelocity(const Entity& entity);
std::optional<glm::dmat4> getTransform(const sim::Entity& entity);

void setPosition(Entity& entity, const Vector3& position);
void setOrientation(Entity& entity, const Quaternion& orientation);
void setVelocity(Entity& entity, const Vector3& velocity);

} // namespace sim

} // namespace skybolt