/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <SkyboltEngine/SkyboltEngineFwd.h>

#include <memory>

namespace pybind11 {
	class scoped_interpreter;
}

namespace skybolt {

class PythonInterpreter
{
public:
	PythonInterpreter(EngineRoot* engineRoot);
	~PythonInterpreter();

private:
	std::unique_ptr<pybind11::scoped_interpreter> mPyInterpreter;
};

} // namespace skybolt