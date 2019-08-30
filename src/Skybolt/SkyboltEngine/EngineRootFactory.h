/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "EngineRoot.h"

namespace skybolt {

class EngineRootFactory
{
public:
	static std::unique_ptr<EngineRoot> create(vis::Window* window, const std::vector<PluginFactory>& pluginFactories, const nlohmann::json& settings);
};

} // namespace skybolt