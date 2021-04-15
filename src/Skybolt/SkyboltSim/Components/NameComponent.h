/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltSim/Component.h"
#include "SkyboltSim/SkyboltSimFwd.h"
#include <SkyboltCommon/Listenable.h>
#include <map>
#include <string>

namespace skybolt {
namespace sim {

struct NamedObjectRegistryListener
{
	virtual void objectAdded(const std::string& name, sim::Entity* entity) {};
	virtual void objectRemoved(const std::string& name, sim::Entity* entity) {};
};

class NamedObjectRegistry : public skybolt::Listenable<NamedObjectRegistryListener>
{
public:
	void add(const std::string& name, sim::Entity* entity);
	void remove(const std::string& name);

	sim::Entity* getObjectByName(const std::string& name) const;

private:
	std::map<std::string, sim::Entity*> namedObjects;
};

class NameComponent : public sim::Component
{
public:
	NameComponent(const std::string& name, const NamedObjectRegistryPtr& registry, sim::Entity* entity)
	: mName(name), mRegistry(registry)
	{
		mRegistry->add(name, entity);
	}

	~NameComponent()
	{
		mRegistry->remove(mName);
	}

	const std::string& getName() const {return mName;}

private:
	std::string mName;
	NamedObjectRegistryPtr mRegistry;
};

const std::string& getName(const sim::Entity& entity);

} // namespace sim
} // namespace skybolt