#pragma once

#include <SkyboltReflect/SkyboltReflectFwd.h>
#include <SkyboltEngine/SkyboltEngineFwd.h>
#include <SkyboltSim/SkyboltSimFwd.h>

#include <pybind11/pybind11.h>
#include <map>
#include <string>

namespace skybolt {

class PyComponentProperty
{
public:
	PyComponentProperty(refl::TypeRegistry* typeRegistry, const sim::ComponentPtr& component, const refl::PropertyPtr& property);

	const std::string& getName() const;

	bool isReadOnly() const;

	pybind11::object getValue();

	void setValue(const pybind11::handle& value);

private:
	refl::TypeRegistry* mTypeRegistry;
	sim::ComponentPtr mComponent;
	refl::PropertyPtr mProperty;
};

using PyComponentPropertyPtr = std::shared_ptr<PyComponentProperty>;

std::map<std::string, PyComponentPropertyPtr> getComponentProperties(skybolt::EngineRoot& engineRoot, const sim::ComponentPtr& component);
PyComponentPropertyPtr getComponentProperty(skybolt::EngineRoot& engineRoot, const sim::ComponentPtr& component, const std::string& name);

} // namespace skybolt