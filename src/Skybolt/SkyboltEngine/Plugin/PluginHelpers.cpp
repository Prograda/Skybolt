/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "PluginHelpers.h"
#include <set>
 
namespace fs = std::filesystem;

namespace skybolt {

// return the filenames of all files that have the specified extension
// in the specified directory and all subdirectories
static std::vector<fs::path> getAllFilesWithExtensions(const fs::path& root, const std::set<std::string>& extensions)
{
	std::vector<fs::path> ret;

	if (!fs::exists(root) || !fs::is_directory(root)) return ret;

	fs::recursive_directory_iterator it(root);
	fs::recursive_directory_iterator endit;

	while (it != endit)
	{
		if (fs::is_regular_file(*it) && extensions.find(it->path().extension().string()) != extensions.end())
		{
			ret.push_back(it->path());
		}
		++it;

	}
	return ret;
}

std::vector<fs::path> getAllPluginFilepathsInDirectory(const std::string& directory)
{
	return getAllFilesWithExtensions(directory, { ".dll", ".dylib", ".dso" });
}

std::vector<std::filesystem::path> getAllPluginFilepathsInDirectories(const std::vector<std::string>& directories)
{
	std::vector<std::filesystem::path> result;
	for (const auto& path : directories)
	{
		auto plugins = getAllPluginFilepathsInDirectory(path);
		result.insert(result.begin(), plugins.begin(), plugins.end());
	}
	return result;
}

} // namespace skybolt