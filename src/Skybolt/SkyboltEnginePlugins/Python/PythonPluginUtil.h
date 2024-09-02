#pragma once

#include <pybind11/pybind11.h>

namespace skybolt {

std::vector<pybind11::module> loadPythonPluginModules(const std::vector<std::string>& moduleNames);

} // namespace skybolt