/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "PythonPluginUtil.h"

#include <boost/log/trivial.hpp>

namespace py = pybind11;

namespace skybolt {

std::vector<pybind11::module> loadPythonPluginModules(const std::vector<std::string>& moduleNames)
{
	std::vector<pybind11::module> result;

	for (const std::string& moduleName : moduleNames)
	{
		pybind11::module module = py::module::import(moduleName.c_str());
		
		if (py::hasattr(module, "skybolt_register"))
		{
			// Module found
			module.attr("skybolt_register")();
			BOOST_LOG_TRIVIAL(info) << "Registered python plugin module: " << moduleName;
		}
		else
		{
			std::string filename = module.attr("__file__")().cast<std::string>();
			throw std::runtime_error("Python plugin '" + filename + "' does not have 'register' function");
		}

		result.push_back(module);
	}
	return result;
}

} // namespace skybolt