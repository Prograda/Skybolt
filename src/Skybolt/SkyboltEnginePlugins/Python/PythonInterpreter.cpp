/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "PythonInterpreter.h"
#include <SkyboltEngine/EngineRoot.h>

#pragma push_macro("slots")
#undef slots
#include <pybind11/embed.h>
#pragma pop_macro("slots")

namespace py = pybind11;

namespace skybolt {

PythonInterpreter::PythonInterpreter(EngineRoot* engineRoot)
{
	try
	{
		if (!Py_IsInitialized()) // If interpreter not already managed externally
		{
			// Configure interpreter with optional SKYBOLT_PYTHON_HOME environment variable
			PyConfig config;
			PyConfig_InitPythonConfig(&config);
			config.install_signal_handlers = 1;

			if (const char* pythonHome = std::getenv("SKYBOLT_PYTHON_HOME"); pythonHome)
			{
				wchar_t* wpath = Py_DecodeLocale(pythonHome, nullptr);
				if (!wpath)
				{
    				throw std::runtime_error("Failed to convert path to wide string");
				}
				PyConfig_SetString(
					&config,
					&config.executable,
					wpath);

				PyMem_RawFree(wpath);
			}

			// Create the interpreter
			mPyInterpreter = std::make_unique<py::scoped_interpreter>(&config);
		}

		py::list sysPath = py::module::import("sys").attr("path").cast<py::list>();

		auto scriptFolders = getPathsInAssetPackages(engineRoot->getAssetPackagePaths(), "Scripts");
		for (const file::Path& scriptFolder : scriptFolders)
		{
			sysPath.append(scriptFolder.string());
		}

		if (const char* pythonPath = std::getenv("SKYBOLT_PYTHON_PATH"); pythonPath)
		{
			for (const std::string& path : file::splitByPathListSeparator(pythonPath))
			{
				sysPath.append(path);
			}
		}

		py::module skyboltModule = py::module::import("skybolt");
		skyboltModule.attr("setGlobalEngineRoot")(engineRoot);
	}
	catch (const pybind11::error_already_set& e)
	{
		// Convert python exception to standard one because it will become invalid after the interpreter is destroyed.
		throw std::runtime_error(e.what());
	}
}

PythonInterpreter::~PythonInterpreter() = default;

} // namespace skybolt