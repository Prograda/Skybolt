/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "Reflection.h"

namespace skybolt::refl {

void Property::addMetadata(const MetadataMap& metadata)
{
	mMetadata.insert(metadata.begin(), metadata.end());
}

void Property::addMetadata(const std::string& name, const std::any& value)
{
	mMetadata[name] = value;
}

std::any Property::getMetadata(const std::string& name) const
{
	if (auto i = mMetadata.find(name); i != mMetadata.end())
	{
		return i->second;
	}
	return {};
}

void Type::addProperty(const PropertyPtr& property)
{
	mProperties[property->getName()] = property;
}

PropertyPtr Type::getProperty(const std::string& name)
{
	if (auto i = mProperties.find(name); i != mProperties.end())
	{
		return i->second;
	}
	for (const auto& [index, type] : mSuperTypes)
	{
		if (const auto& property = type.first->getProperty(name); property)
		{
			return property;
		}
	}
	return nullptr;
}

Type::PropertyMap Type::getProperties() const
{
	Type::PropertyMap r = mProperties;
	for (const auto& [index, type] : mSuperTypes)
	{
		const auto& superTypeProperties = type.first->getProperties();
		r.insert(superTypeProperties.begin(), superTypeProperties.end());
	}
	return r;
}

void Type::addSuperType(const TypePtr& super, std::ptrdiff_t offsetFromThisToSuper)
{
	mSuperTypes[super->getTypeIndex()] = {super, offsetFromThisToSuper};
}

std::optional<std::ptrdiff_t> Type::getOffsetFromThisToSuper(const std::type_index& super) const
{
	// Look for direct super types
	if (auto i = mSuperTypes.find(super); i != mSuperTypes.end())
	{
		return i->second.second;
	}

	// Walk tree to find indirect super types
	for (const auto& [typeIndex, typeAndOffset] : mSuperTypes)
	{
		if (auto offset = typeAndOffset.first->getOffsetFromThisToSuper(super); offset)
		{
			return typeAndOffset.second + *offset;
		}
	}

	// No super type found
	return std::nullopt;
}

static std::vector<RegistrationHandler>& getGlobalHandlers()
{
	static std::vector<RegistrationHandler> r;
	return r;
}

void addStaticRegistrationHandler(RegistrationHandler handler)
{
	getGlobalHandlers().emplace_back(std::move(handler));
}

void addStaticallyRegisteredTypes(TypeRegistry& registry)
{
	std::vector<TypeDefinitionRegistrationHandler> typeDefinitionRegistrationHandlers;
	TypeRegistryBuilder builder(registry, [&] (TypeDefinitionRegistrationHandler registry) {
		typeDefinitionRegistrationHandlers.push_back(registry);
	});

	// Register types
	for (const auto& handler : getGlobalHandlers())
	{
		handler(builder);
	}

	// Perform delayed registration of type definitions. This is performed as a second step
	// so that types are registered for properties to refer to.
	for (const auto& i : typeDefinitionRegistrationHandlers)
	{
		i(registry);
	}
}

TypeRegistry::TypeRegistry()
{
	addStaticallyRegisteredTypes(*this);
}

void TypeRegistry::addType(const TypePtr& type)
{
	mTypesByName[type->getName()] = type;
	mTypesByTypeIndex[type->getTypeIndex()] = type;
}

TypePtr TypeRegistry::getTypeByName(const std::string& name) const
{
	if (auto i = mTypesByName.find(name); i != mTypesByName.end())
	{
		return i->second;
	}
	return nullptr;
}

} // namespace skybolt::refl