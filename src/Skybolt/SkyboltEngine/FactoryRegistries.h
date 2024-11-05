/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

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