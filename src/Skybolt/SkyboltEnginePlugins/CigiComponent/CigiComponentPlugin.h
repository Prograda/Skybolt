/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <SkyboltEngine/ComponentFactory.h>
#include <SkyboltEngine/Plugin/Plugin.h>

namespace skybolt {

class CigiComponentPlugin : public Plugin
{
public:
	CigiComponentPlugin(const PluginConfig& config);

	~CigiComponentPlugin() override;

private:
	ComponentFactoryRegistryPtr mComponentFactoryRegistry;
};

namespace plugins {
std::shared_ptr<Plugin> createCigiComponentPlugin(const PluginConfig& config);
}

} // namespace skybolt