/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "PythonFunction.h"
#include "Nodes/NodeDataT.h"
#include <SkyboltCommon/Exception.h>

#pragma push_macro("slots")
#undef slots
#include <pybind11/eval.h>
#pragma pop_macro("slots")
#include "object.h"

using namespace skybolt;
namespace py = pybind11;
using namespace py::literals;

PythonFunction::PythonFunction(const NodeDefPtr& nodeDef) :
	FlowFunction(nodeDef)
{
}

void PythonFunction::setCode(const std::string& code)
{
	mCode = code;

	mCompiledCode.reset();

	if (!mCode.empty())
	{
		PyObject* object = Py_CompileString(code.c_str(), getName().c_str(), Py_file_input);
		if (object)
		{
			mCompiledCode.reset(object, [](PyObject* p) { Py_DECREF(p); });
		}
		else
		{
			throw py::error_already_set();
		}
	}
}

NodeDataPtrVector PythonFunction::eval(const NodeDataPtrVector& inputs) const
{
	if (!mCompiledCode)
	{
		return {};
	}

	NodeDataPtrVector outputs;

	// Set inputs
	py::dict pyInputs = py::dict();
	int i = 0;
	for (const NodeDataPtr& input : inputs)
	{
		std::string name = getNodeDef()->inputs[i].name.toStdString();
		if (DoubleNodeData* data = dynamic_cast<DoubleNodeData*>(input.get()))
		{
			pyInputs[name.c_str()] = data->data;
		}
		++i;
	}

	// Create outputs
	py::dict pyOutputs = py::dict();
	for (const QtNodes::NodeDataType& output : getNodeDef()->outputs)
	{
		std::string name = output.name.toStdString();
		if (output.id == DoubleNodeData::typeId())
		{
			pyOutputs[name.c_str()] = 0.0;
		}
	}

	// Execute
	py::object global = py::globals();
	py::object local = py::dict("inputs"_a = pyInputs, "outputs"_a = pyOutputs);
	PyObject* result = PyEval_EvalCode(mCompiledCode.get(), global.ptr(), local.ptr());
	if (!result)
	{
		throw py::error_already_set();
	}

	// Copy output values from python to C++
	for (const QtNodes::NodeDataType& output : getNodeDef()->outputs)
	{
		const QString& name = output.name;
		py::object pyOutput = pyOutputs[name.toStdString().c_str()];
		if (output.id == DoubleNodeData::typeId())
		{
			outputs.push_back(std::make_shared<DoubleNodeData>(name, pyOutput.cast<double>()));
		}
	}

	return outputs;
}
