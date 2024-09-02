/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "PyComponent.h"
#include "PythonBindings.h"

#include <SkyboltEngine/EngineRoot.h>

namespace py = pybind11;

namespace skybolt {

using namespace skybolt::sim;

static py::detail::str_attr_accessor getRequiredAttr(const py::handle& obj, const char* name)
{
	if (!py::hasattr(obj, name))
	{
		throw std::runtime_error("Could not find required python attribute: " + std::string(name));
	}
	return obj.attr(name);
}

PyComponent::PyComponent(py::object pythonComponent) :
	mPythonComponent(std::move(pythonComponent))
{
	EngineRoot* engine = getGlobalEngineRoot();
	if (!engine)
	{
		throw std::runtime_error("Global EngineRoot not instantiated");
	}

	refl::TypeRegistry* typeRegistry = engine->typeRegistry.get();

	auto properties = getRequiredAttr(mPythonComponent, "properties");
	mPropertiesDict = properties.attr("__dict__");
	for (const auto& [name, property] : mPropertiesDict)
	{
		addProperty(*typeRegistry, py::cast<std::string>(name), property);
	}
}

PyComponent::~PyComponent() = default;

refl::Type::PropertyMap PyComponent::getProperties() const
{
	return mProperties;
}

void PyComponent::setSimTime(SecondsD newTime)
{
	if (pybind11::hasattr(mPythonComponent, "set_sim_time"))
	{
		mPythonComponent.attr("set_sim_time")(newTime);
	}
}

void PyComponent::advanceSimTime(SecondsD newTime, SecondsD dt)
{
	if (pybind11::hasattr(mPythonComponent, "advance_sim_time"))
	{
		mPythonComponent.attr("advance_sim_time")(newTime, dt);
	}
}

void PyComponent::addProperty(refl::TypeRegistry& typeRegistry, const std::string& name, const py::handle& value)
{
	if (py::isinstance<py::bool_>(value))
	{
		addPropertyOfType(typeRegistry, name, value.cast<bool>());
	}
	else if (py::isinstance<py::float_>(value))
	{
		addPropertyOfType(typeRegistry, name, value.cast<double>());
	}
	if (py::isinstance<py::int_>(value))
	{
		addPropertyOfType(typeRegistry, name, value.cast<int>());
	}
	if (py::isinstance<py::str>(value))
	{
		addPropertyOfType(typeRegistry, name, value.cast<std::string>());
	}
}

} // namespace skybolt