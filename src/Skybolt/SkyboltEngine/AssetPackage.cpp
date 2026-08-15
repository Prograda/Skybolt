/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "AssetPackage.h"
#include <SkyboltCommon/Logging/Logging.h>
#include <SkyboltCommon/Json/JsonHelpers.h>
#include <SkyboltCommon/Json/ReadJsonFile.h>

#include <osgDB/Registry>

namespace skybolt {

struct AssetPackageDefinition
{
	std::string name;
	file::Path filepath;

	//! Assets in packages with lower priority order number will override
	//! other assets with same asset-package-relative filename.
	int defaultPriorityOrder = 0;
};

static AssetPackageDefinition readAssetPackageDefinition(const nlohmann::json& j)
{
	return AssetPackageDefinition{
		.defaultPriorityOrder = readOptional<int>(j, "defaultPriorityOrder").value_or(100)
	};
} 

static void registerAssetPackage(const AssetPackageDefinition& definition)
{
	osgDB::Registry::instance()->getDataFilePathList().push_back(definition.filepath.string() + "/");
	SKYBOLT_LOG(info) << "Registered asset package: " << definition.name;
}

AssetPackageFilepaths loadAssetPackagesInPath(const file::Path& assetSearchPath)
{
	// Find packages
	std::vector<AssetPackageDefinition> definitions;
	file::Paths folders = file::findFoldersInDirectory(assetSearchPath.string());
	for (const auto& folder : folders)
	{
		AssetPackageDefinition definition;
		const file::Path packageJsonFilename = folder / "package.json";
		if (!std::filesystem::exists(packageJsonFilename))
		{
			SKYBOLT_LOG(warning) << "Folder on asset package search path has no package.json file: " << packageJsonFilename << ". Folders without a package.json file are currently treated as packages regardless, but may be ignored in future Skybolt versions.";
		}
		else
		{
			nlohmann::json j = readJsonFile(packageJsonFilename.string());
			definition = readAssetPackageDefinition(j);
		}
		definition.name = folder.stem().string();
		definition.filepath = folder;
		definitions.push_back(definition);
	}

	// Sort by priority order
	std::sort(definitions.begin(), definitions.end(), [](const auto& a, const auto& b) {
		return a.defaultPriorityOrder < b.defaultPriorityOrder;
	});

	// Load packages
	AssetPackageFilepaths loadedPackagePaths;
	for (const AssetPackageDefinition& definition : definitions)
	{
		registerAssetPackage(definition);
		loadedPackagePaths[definition.name] = definition.filepath;

	}
	return loadedPackagePaths;
}

file::Paths getPathsInAssetPackages(const std::vector<std::string>& assetPackagePaths, const std::string& relativePath)
{
	file::Paths result;
	for (const auto& packagePath : assetPackagePaths)
	{
		std::string path = packagePath + "/" + relativePath;
		if (std::filesystem::exists(path))
		{
			result.push_back(path);
		}
	}
	return result;
}

file::Paths getFilesWithExtensionInDirectoryInAssetPackages(const std::vector<std::string>& assetPackagePaths, const std::string& relativeDirectory, const std::string& extension)
{
	int levels = 0; // Do not recurse into subdirectories
	return getFilesWithExtensionInDirectoryInAssetPackagesRecursive(assetPackagePaths, relativeDirectory, extension, levels);
}

static file::Paths removeDuplicates(const file::Paths& paths)
{
	std::set<file::Path> uniquePaths(paths.begin(), paths.end());
	return {uniquePaths.begin(), uniquePaths.end()};
}

file::Paths getFilesWithExtensionInDirectoryInAssetPackagesRecursive(const std::vector<std::string>& assetPackagePaths, const std::string& relativeDirectory, const std::string& extension, std::optional<int> depth)
{
	file::Paths result;
	for (const auto& packagePath : assetPackagePaths)
	{
		std::string path = packagePath + "/" + relativeDirectory;
		if (std::filesystem::exists(path))
		{
			auto paths = file::findFilenamesInDirectoryRecursive(path, extension, depth.value_or(100));
			result.insert(result.end(), paths.begin(), paths.end());
		}
	}
	result = removeDuplicates(result);
	return result;
}

} // namespace skybolt