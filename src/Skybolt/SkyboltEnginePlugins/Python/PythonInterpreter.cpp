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

PythonInterpreter::PythonInterpreter(EngineRoot* engineRoot) :
	mPyInterpreter(std::make_unique<py::scoped_interpreter>())
{
	try
	{
		py::list sysPath = py::module::import("sys").attr("path").cast<py::list>();

		auto scriptFolders = getPathsInAssetPackages(engineRoot->getAssetPackagePaths(), "Scripts");
		for (const file::Path& scriptFolder : scriptFolders)
		{
			sysPath.append(scriptFolder.string());
		}

		if (const char* skyboltLibPath = std::getenv("SKYBOLT_LIB_PATH"); skyboltLibPath)
		{
			for (const std::string& path : file::splitByPathListSeparator(skyboltLibPath))
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