/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <SkyboltCommon/Exception.h>
#include <SkyboltCommon/Registry.h>
#include <SkyboltCommon/TypedItemContainer.h>

#include <functional>

namespace skybolt {
namespace vis {

class VisFactory
{
public:
	virtual ~VisFactory() = default;

	virtual std::vector<std::type_index> getExposedTypes() const = 0;
};

template <class DerivedT>
class VisFactoryT : public VisFactory
{
public:
	~VisFactoryT() override = default;

	std::vector<std::type_index> getExposedTypes() const override
	{
		return { typeid(DerivedT), typeid(VisFactory) };
	}
};

template <class ResultT, typename... CreationArgsT>
class VisFactoryFunction : public VisFactoryT<VisFactoryFunction<ResultT, CreationArgsT...>>
{
public:
	using Fn = std::function<std::unique_ptr<ResultT>(CreationArgsT...)>;
	VisFactoryFunction(Fn fn) : mFn(std::move(fn)) {}
	~VisFactoryFunction() override = default;

	std::unique_ptr<ResultT> create(CreationArgsT... args)
	{
		return mFn(args...);
	}

private:
	Fn mFn;
};

struct VisFactoryRegistry : TypedItemContainer<VisFactory>, Registry {};
using VisFactoryRegistryPtr = std::shared_ptr<VisFactoryRegistry>;

template <typename T>
std::shared_ptr<T> findFactoryRequired(const VisFactoryRegistry& registry)
{
	if (const auto& factory = registry.getFirstItemOfType<T>(); factory)
	{
		return factory;
	}
	throw Exception("Could not find factory of type '" + std::string(typeid(T).name()) + "'");
}

} // namespace vis

template<>
inline std::vector<std::type_index> getExposedTypes<vis::VisFactory>(const vis::VisFactory& factory)
{
	return factory.getExposedTypes();
}

} // namespace skybolt
