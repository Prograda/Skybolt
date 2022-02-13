/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "EntityVisibilityFilterable.h"
#include "SimVisBinding.h"
#include <SkyboltSim/World.h>

#include <osg/Switch>
#include <osg/MatrixTransform>

namespace skybolt {

 //! Binds each sim object in the world to a vis object of type T.
 //! Visibility of vis objects is controlled by a predicate.
template <typename T>
class SimVisObjectsReflector : public sim::WorldListener, public EntityVisibilityFilterable
{
public:
	SimVisObjectsReflector(sim::World* world, osg::Group* parent) :
		mWorld(world),
		mGroup(new osg::Switch),
		mVisibilityPredicate(visibilityOn)
	{
		mWorld->addListener(this);

		parent->addChild(mGroup);
	}

	~SimVisObjectsReflector()
	{
		mGroup->getParent(0)->removeChild(mGroup);
		mWorld->removeListener(this);
	}

	void setVisibilityPredicate(VisibilityPredicate predicate) override
	{
		mVisibilityPredicate = predicate;
	}

protected:
	virtual std::optional<T> createObject(const sim::EntityPtr& entity) = 0;
	virtual void destroyObject(const T& object) = 0;
	virtual osg::Node* getNode(const T& object) const = 0;

	bool applyVisibility(const sim::Entity& entity, const T& visObject)
	{
		bool visibility = mVisibilityPredicate(entity);
		mGroup->setChildValue(getNode(visObject), visibility);
		return visibility;
	}

	const std::map<sim::Entity*, T>& getObjectsMap() const { return mEntities; }

private:
	void entityAdded(const sim::EntityPtr& entity) override
	{
		std::optional<T> optionalVisObject = createObject(entity);
		if (optionalVisObject)
		{
			const T& visObject = *optionalVisObject;

			mEntities[entity.get()] = visObject;
			mGroup->addChild(getNode(visObject));
		}
	}

	void entityAboutToBeRemoved(const sim::EntityPtr& entity) override
	{
		auto it = mEntities.find(entity.get());
		if (it != mEntities.end())
		{
			mGroup->removeChild(getNode(it->second));
			mEntities.erase(it);
		}
	}

protected:
	osg::Switch* mGroup;

private:
	sim::World* mWorld;
	std::map<sim::Entity*, T> mEntities;
	VisibilityPredicate mVisibilityPredicate;
};

} // namespace skybolt