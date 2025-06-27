#pragma once

#include <string>
#include <vector>

//! @returns a vector of names of missing dependency libraries
std::vector<std::string> getMissingDependencies(const std::string& dllPath);