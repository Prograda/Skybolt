/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <SkyboltSim/Component.h>

#include <pybind11/pybind11.h>

namespace skybolt {

//! Wrapper allowing Python scripts define Component functionality
class PyComponent : public sim::Component, public refl::DynamicPropertySource
{
public:
	PyComponent(pybind11::object pythonComponent);
	~PyComponent() override;

	refl::Type::PropertyMap getProperties() const override;

    void setSimTime(sim::SecondsD newTime) override;
	void advanceSimTime(sim::SecondsD newTime, sim::SecondsD dt) override;

private:
	void addProperty(refl::TypeRegistry& typeRegistry, const std::string& name, const pybind11::handle& value);

	template <typename T>
	void addPropertyOfType(refl::TypeRegistry& typeRegistry, const std::string& name, const T& value)
	{
		auto getter = [name] (const PyComponent& c) -> T {
			return pybind11::cast<T>(c.mPropertiesDict[name.c_str()]);
		};

		auto setter = [name, this] (PyComponent& c, const T& value) {
			auto oldValue = c.mPropertiesDict[name.c_str()];
			if (pybind11::cast<T>(oldValue) != value)
			{
				c.mPropertiesDict[name.c_str()] = value;
				if (pybind11::hasattr(mPythonComponent, "property_changed"))
				{
					mPythonComponent.attr("property_changed")(name, value);
				}
			}
		};

		mProperties[name] = std::make_shared<refl::GetterSetterFunctionProperty<PyComponent, T, const T&>>(&typeRegistry, name, typeRegistry.getOrCreateType<T>(), getter, setter);
	}

private:
	pybind11::object mPythonComponent;
	pybind11::dict mPropertiesDict;
	refl::Type::PropertyMap mProperties;
};

SKYBOLT_REFLECT_BEGIN(PyComponent)
{
	registry.type<PyComponent>("PyComponent")
		.superType<sim::Component>()
		.superType<refl::DynamicPropertySource>();
}
SKYBOLT_REFLECT_END

} // namespace skybolt