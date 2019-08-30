/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SprocketFwd.h"
#include <SkyboltEngine/EngineRoot.h>

#include <functional>
#include <memory>

class SprocketModel
{
public:
	SprocketModel(skybolt::EngineRoot* root, skybolt::InputPlatform* inputPlatform);
	~SprocketModel();

	void addFileSearchPath(const std::string& path) { mFileSearchPaths.insert(path); }
	void removeFileSearchPath(const std::string& path) { mFileSearchPaths.erase(path); }

	skybolt::EngineRoot* engineRoot;
	skybolt::file::FileLocator fileLocator;

private:
	std::set<std::string> mFileSearchPaths;
	skybolt::InputPlatform* mInputPlatform;
};
