/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltEngine/SkyboltEngineFwd.h"
#include "SkyboltSim/SkyboltSimFwd.h"
#include "SkyboltSim/System/SystemRegistry.h"
#include "SkyboltEngine/ComponentFactory.h"
#include <SkyboltVis/VisFactory.h>

#include <boost/dll/import.hpp>

namespace skybolt {

struct PluginConfig
{
	EngineRoot* engineRoot; //!< EngineRoot lifetime is guaranteed to exceed plugin lifetime
	vis::VisFactoryRegistryPtr visFactoryRegistry;
	ComponentFactoryRegistryPtr simComponentFactoryRegistry;
};

class BOOST_SYMBOL_VISIBLE Plugin
{
public:
	static std::string factorySymbolName() { return "createEnginePlugin"; }

	virtual ~Plugin() {}

	virtual void update() {}
};

} // namespace skybolt