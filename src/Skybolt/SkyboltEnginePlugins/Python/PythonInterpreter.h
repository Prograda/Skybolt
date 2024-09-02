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