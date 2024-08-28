#pragma once

#include <SkyboltCommon/Expected.h>
#include <SkyboltCommon/TypedItemContainer.h>

namespace skybolt {

using FactoryRegistries = TypedItemContainer<Registry>;

template <class T>
Expected<std::shared_ptr<T>> getExpectedRegistry(const FactoryRegistries& registries)
{
	auto registry = registries.getFirstItemOfType<T>();
	if (!registry)
	{
		return UnexpectedMessage{ "Could not find factory registry of type: " + std::string(typeid(T).name()) };
	}
	return registry;
}

} // namespace skybolt