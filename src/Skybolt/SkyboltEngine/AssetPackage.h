#pragma once

#include <SkyboltCommon/File/FileUtility.h>
#include <map>
#include <vector>

namespace skybolt {

using AssetPackageName = std::string;
using AssetPackageFilepaths = std::map<AssetPackageName, file::Path>;
AssetPackageFilepaths loadAssetPackagesInPath(const file::Path& assetSearchPath);

file::Paths getPathsInAssetPackages(const std::vector<std::string>& assetPackagePaths, const std::string& relativePath);

file::Paths getFilesWithExtensionInDirectoryInAssetPackages(const std::vector<std::string>& assetPackagePaths, const std::string& relativeDirectory, const std::string& extension);

//! @param depth specifies number of levels to search recurisvely. A depth of 1 will search down one level below the given directory. If unspecified, recursion is effectivly infinite, or some large number.
file::Paths getFilesWithExtensionInDirectoryInAssetPackagesRecursive(const std::vector<std::string>& assetPackagePaths, const std::string& relativeDirectory, const std::string& extension, std::optional<int> depth);

} // namespace skybolt