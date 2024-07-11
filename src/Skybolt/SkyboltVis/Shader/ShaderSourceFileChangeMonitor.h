/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "ShaderProgramRegistry.h"
#include <osg/Program>
#include <memory>
#include <mutex>
#include <set>
#include <vector>

namespace skybolt {
namespace vis {

//! Monitors shader files and reloads file and associated program if the file changes on disk
class ShaderSourceFileChangeMonitor
{
public:
	ShaderSourceFileChangeMonitor(const ShaderPrograms& programs);
	~ShaderSourceFileChangeMonitor();

	void update();

private:
	void reloadShadersUsingFile(const std::string& filename);

private:
	ShaderPrograms mPrograms;
	std::mutex mChangedFilesMutex;
	std::set<std::string> mChangedFiles;
	std::vector<std::shared_ptr<struct WatcherImpl>> mWatchers;
};

} // namespace vis
} // namespace skybolt
