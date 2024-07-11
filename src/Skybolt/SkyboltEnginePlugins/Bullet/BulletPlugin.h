/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <SkyboltEngine/Plugin/Plugin.h>

namespace skybolt {

namespace sim { class BulletWorld; }

class BulletPlugin : public Plugin
{
public:
	BulletPlugin(const PluginConfig& config);

	~BulletPlugin() override;

private:
	sim::SystemPtr mBulletSystem;
	std::unique_ptr<sim::BulletWorld> mBulletWorld;
	ComponentFactoryRegistryPtr mComponentFactoryRegistry;
	sim::SystemRegistryPtr mSystemRegistry;
};

namespace plugins {

	std::shared_ptr<Plugin> createBulletPlugin(const PluginConfig& config);
}

} // namespace skybolt {