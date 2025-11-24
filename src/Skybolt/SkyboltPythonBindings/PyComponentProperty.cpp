#include "PyComponentProperty.h"

#include <SkyboltEngine/EngineRoot.h>
#include <SkyboltReflect/Reflection.h>
#include <SkyboltSim/Component.h>

#include <boost/log/trivial.hpp>

namespace py = pybind11;

namespace skybolt {

using namespace skybolt::sim;

using ReflInstanceToPyObject = std::function<py::object(const refl::Instance& instance)>;

template <typename CppTypeT, typename PythonTypeT>
ReflInstanceToPyObject createReflInstanceToPyObject()
{
	return [] (const refl::Instance& instance) -> py::object {
		const auto& value = instance.cast<CppTypeT>();
		return PythonTypeT(value);
		};
}

static py::object reflInstanceToPyObject(refl::TypeRegistry& registry, const refl::Instance& instance)
{
	static std::map<const refl::Type*, ReflInstanceToPyObject> translators = {
		{ registry.getOrCreateType<bool>().get(), createReflInstanceToPyObject<bool, py::bool_>() },
		{ registry.getOrCreateType<int>().get(), createReflInstanceToPyObject<int, py::int_>() },
		{ registry.getOrCreateType<unsigned int>().get(), createReflInstanceToPyObject<unsigned int, py::int_>() },
		{ registry.getOrCreateType<float>().get(), createReflInstanceToPyObject<float, py::float_>() },
		{ registry.getOrCreateType<double>().get(), createReflInstanceToPyObject<double, py::float_>() },
		{ registry.getOrCreateType<std::string>().get(), createReflInstanceToPyObject<std::string, py::str>() }
	};

	if (const auto& i = translators.find(instance.getType().get()); i != translators.end())
	{
		return (i->second)(instance);
	}
	return {};
}

using PyHandleToReflInstance = std::function<std::optional<refl::Instance>(refl::TypeRegistry& typeRegistry, const py::handle& value)>;

template <typename CppTypeT>
PyHandleToReflInstance createPyHandleToReflInstance()
{
	return [] (refl::TypeRegistry& typeRegistry, const py::handle& value) -> refl::Instance {
		return refl::makeValueInstance(typeRegistry, value.cast<CppTypeT>());
		};
}

static std::optional<refl::Instance> pyHandleToReflInstance(refl::TypeRegistry& registry, const refl::TypePtr& type, const py::handle& value)
{
	assert(type);

	static std::map<const refl::Type*, PyHandleToReflInstance> translators = {
		{ registry.getOrCreateType<bool>().get(), createPyHandleToReflInstance<bool>() },
		{ registry.getOrCreateType<int>().get(), createPyHandleToReflInstance<int>() },
		{ registry.getOrCreateType<unsigned int>().get(), createPyHandleToReflInstance<unsigned int>() },
		{ registry.getOrCreateType<float>().get(), createPyHandleToReflInstance<float>() },
		{ registry.getOrCreateType<double>().get(), createPyHandleToReflInstance<double>() },
		{ registry.getOrCreateType<std::string>().get(), createPyHandleToReflInstance<std::string>() }
	};

	if (const auto& i = translators.find(type.get()); i != translators.end())
	{
		return (i->second)(registry, value);
	}
	return std::nullopt;
}

PyComponentProperty::PyComponentProperty(refl::TypeRegistry* typeRegistry, const ComponentPtr& component, const refl::PropertyPtr& property) :
	mTypeRegistry(typeRegistry),
	mComponent(component),
	mProperty(property)
{
	assert(mTypeRegistry);
	assert(mComponent);
	assert(mProperty);
}

const std::string& PyComponentProperty::getName() const { return mProperty->getName(); }

bool PyComponentProperty::isReadOnly() const { return mProperty->isReadOnly(); }

py::object PyComponentProperty::getValue()
{
	auto objectInstance = refl::makeRefInstance(*mTypeRegistry, mComponent.get());
	auto valueInstance = mProperty->getValue(objectInstance);
	return reflInstanceToPyObject(*mTypeRegistry, valueInstance);
}

void PyComponentProperty::setValue(const py::handle& value)
{
	auto valueInstance = pyHandleToReflInstance(*mTypeRegistry, mProperty->getType(), value);
	if (!valueInstance)
	{
		BOOST_LOG_TRIVIAL(error) << "Could not set property '" << mProperty->getName() << "' from python because data type '" << mProperty->getType()->getName() << "' is not supported";
		return;
	}

	auto objectInstance = refl::makeRefInstance(*mTypeRegistry, mComponent.get());
	mProperty->setValue(objectInstance, *valueInstance);
}

std::map<std::string, PyComponentPropertyPtr> getComponentProperties(EngineRoot& engineRoot, const ComponentPtr& component)
{
	refl::Instance instance = refl::makeRefInstance(*engineRoot.typeRegistry, component.get());
	auto properties = refl::getProperties(instance);

	std::map<std::string, PyComponentPropertyPtr> result;
	for (const auto& property : properties)
	{
		result[property.first] = std::make_shared<PyComponentProperty>(engineRoot.typeRegistry.get(), component, property.second);
	}
	return result;
}

PyComponentPropertyPtr getComponentProperty(EngineRoot& engineRoot, const ComponentPtr& component, const std::string& name)
{
	refl::Instance instance = refl::makeRefInstance(*engineRoot.typeRegistry, component.get());
	auto properties = refl::getProperties(instance);
	if (auto i = properties.find(name); i != properties.end())
	{
		return std::make_shared<PyComponentProperty>(engineRoot.typeRegistry.get(), component, i->second);
	}
	return nullptr;
}

} // namespace skybolt