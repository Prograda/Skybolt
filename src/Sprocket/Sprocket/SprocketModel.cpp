/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "SprocketModel.h"
#include <SkyboltSim/Entity.h>

#include <filesystem>

using namespace skybolt;

SprocketModel::SprocketModel(EngineRoot* root, InputPlatform* inputPlatform) :
	engineRoot(root),
	mInputPlatform(inputPlatform)
{
	mFileSearchPaths.insert("");

	fileLocator = [this](const std::string& filename, file::FileLocatorMode mode) {
		for (const std::string& path : mFileSearchPaths)
		{
			file::Path p(path);
			file::Path f = p.append(filename);

			if (std::filesystem::exists(f))
			{
				return f;
			}
		}
		if (mode == file::FileLocatorMode::Required)
		{
			throw std::runtime_error("Could not find file '" + filename + "'");
		}
		return file::Path();
	};
}

SprocketModel::~SprocketModel()
{
}
