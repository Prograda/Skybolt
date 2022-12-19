/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltEngine/SkyboltEngineFwd.h"
#include <SkyboltSim/SkyboltSimFwd.h>
#include <nlohmann/json.hpp>
#include <px_sched/px_sched.h>

namespace skybolt {

struct ComponentFactoryContext
{
	px_sched::Scheduler* scheduler;
	sim::World* simWorld;
	EntityFactory const* entityFactory;
	JulianDateProvider julianDateProvider;
	EngineStats* stats;
};

class ComponentFactory
{
public:
	virtual ~ComponentFactory() {}

	//! @return nullptr if component could not be created
	virtual sim::ComponentPtr create(sim::Entity* entity, const ComponentFactoryContext& context, const nlohmann::json& json) = 0;
};

class ComponentFactoryFunctionAdapter : public ComponentFactory
{
public:
	typedef std::function<sim::ComponentPtr(sim::Entity* entity, const ComponentFactoryContext& context, const nlohmann::json& json)> Function;
	
	ComponentFactoryFunctionAdapter(Function fn) : mFunction(fn) {}

	sim::ComponentPtr create(sim::Entity* entity, const ComponentFactoryContext& context, const nlohmann::json& json) override
	{
		return mFunction(entity, context, json);
	}

private:
	Function mFunction;
};

typedef std::map<std::string, ComponentFactoryPtr> ComponentFactoryRegistry;
typedef std::shared_ptr<ComponentFactoryRegistry> ComponentFactoryRegistryPtr;

void addDefaultFactories(ComponentFactoryRegistry& registry);

} // namespace skybolt
