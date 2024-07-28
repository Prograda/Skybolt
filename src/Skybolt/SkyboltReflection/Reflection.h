/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltReflectionFwd.h"

#include <any>
#include <assert.h>
#include <functional>
#include <map>
#include <optional>
#include <stdexcept>
#include <string>
#include <typeindex>

namespace skybolt::refl {

class Type
{
public:
	Type(const std::string& name, const std::type_index& typeIndex) :
		mName(name),
		mTypeIndex(typeIndex)
	{}

	void addProperty(const PropertyPtr& property);
	PropertyPtr getProperty(const std::string& name);

	using PropertyMap = std::map<std::string, PropertyPtr>;
	PropertyMap getProperties() const;

	void addSuperType(const TypePtr&, std::ptrdiff_t offsetFromThisToSuper);
	std::optional<std::ptrdiff_t> getOffsetFromThisToSuper(const std::type_index& super) const;

	template <typename T>
	bool isDerivedFrom() const
	{
		// Look for direct super type
		if (auto i = mSuperTypes.find(typeid(T)); i != mSuperTypes.end())
		{
			return true;
		}
		
		// Look for indirect super types
		for (const auto& [typeIndex, typeAndOffset] : mSuperTypes)
		{
			const TypePtr& superType = typeAndOffset.first;
			if (auto result = superType->isDerivedFrom<T>(); result)
			{
				return true;
			}
		}
		return false;
	}

	const std::string& getName() const { return mName; }
	const std::type_index& getTypeIndex() const { return mTypeIndex; }

private:
	std::string mName;
	std::type_index mTypeIndex;
	PropertyMap mProperties;
	std::map<std::type_index, std::pair<TypePtr, std::ptrdiff_t>> mSuperTypes;
};

class TypeRegistry
{
public:
	TypeRegistry();
	void addType(const TypePtr& type);

	TypePtr getTypeByName(const std::string& name) const;

	TypePtr getTypeByIndex(const std::type_index& index) const
	{
		if (auto i = mTypesByTypeIndex.find(index); i != mTypesByTypeIndex.end())
		{
			return i->second;
		}
		return nullptr;
	}

	template <typename T>
	TypePtr getType() const
	{
		return getTypeByIndex(typeid(T));
	}

	template <typename T>
	TypePtr getMostDerivedType(const T& object) const
	{
		return getTypeByIndex(typeid(object));
	}

	template <typename T>
	TypePtr getTypeRequired() const
	{
		if (auto type = getType<T>(); type)
		{
			return type;
		}
		throw std::runtime_error("Could not find type " + std::string(typeid(T).name()));
	}

	TypePtr getOrCreateTypeByIndex(const std::type_index& index)
	{
		if (TypePtr t = getTypeByIndex(index); t)
		{
			return t;
		}
		auto type = std::make_shared<Type>(index.name(), index);
		addType(type);
		return type;
	}

	template <typename T>
	TypePtr getOrCreateType()
	{
		return getOrCreateTypeByIndex(typeid(T));
	}

	template <typename T>
	TypePtr getOrCreateMostDerivedType(const T& object)
	{
		return getOrCreateTypeByIndex(typeid(object));
	}

private:
	std::map<std::string, TypePtr> mTypesByName;
	std::map<std::type_index, TypePtr> mTypesByTypeIndex;
};

template <typename T>
T* addPointerByteOffset(T* p, std::ptrdiff_t offset)
{
	return reinterpret_cast<T*>(reinterpret_cast<unsigned char*>(p) + offset);
}

//! Instance of a Type
class Instance
{
public:
	Instance(TypeRegistry* typeRegistry, std::shared_ptr<void> objectPtr, TypePtr type) :
		mTypeRegistry(typeRegistry),
		mObjectPtr(std::move(objectPtr)),
		mType(std::move(type))
	{
		assert(mTypeRegistry);
		assert(mObjectPtr);
	}

	TypePtr getType() const { return mType; }

	template <typename T>
	T* getObject()
	{
		return const_cast<T*>(static_cast<const Instance*>(this)->getObject<T>());
	}

	template <typename T>
	const T* getObject() const
	{
		std::type_index resultTypeIndex = typeid(T);
		if (resultTypeIndex == mType->getTypeIndex()) // if result type is same type
		{
			return static_cast<T*>(mObjectPtr.get());
		}
		else if (auto offset = mType->getOffsetFromThisToSuper(resultTypeIndex); offset) // if result type is a super type
		{
			return static_cast<T*>(addPointerByteOffset(mObjectPtr.get(), *offset));
		}

		throw std::runtime_error("Instance::getObject() could not cast from " + mType->getName() + " to " + resultTypeIndex.name());
	}

private:
	TypeRegistry* mTypeRegistry;
	std::shared_ptr<void> mObjectPtr;
	TypePtr mType;
};

template <typename T>
inline void null_deleter(T*) {};

template <typename T>
Instance createNonOwningInstance(TypeRegistry* registry, T* object)
{
	TypePtr type = registry->getOrCreateType<T>();
	TypePtr derivedType = registry->getOrCreateMostDerivedType(*object);
	if (type != derivedType)
	{
		if (auto offset = derivedType->getOffsetFromThisToSuper(type->getTypeIndex()); offset)
		{
			void* derivedPointer = addPointerByteOffset(object, -*offset);
			return Instance(registry, std::shared_ptr<void>(derivedPointer, &null_deleter<void>), derivedType);
		}
	}
	return Instance(registry, std::shared_ptr<T>(object, &null_deleter<T>), type);
}

template <typename T>
Instance createOwningInstance(TypeRegistry* registry, T object)
{
	return Instance(registry, std::make_shared<T>(std::move(object)), registry->getOrCreateType<T>());
}

class Property
{
public:
	Property(const std::string& name, const TypePtr& type) :
		mType(type),
		mName(name)
	{}

	virtual ~Property() = default;

	const std::string& getName() const { return mName; }

	const TypePtr& getType() const { return mType; }

	virtual bool setValue(Instance& obj, const Instance& value) = 0; //!< @returns true if value was set correctly
	virtual Instance getValue(const Instance& obj) const = 0;

	void setReadOnly(bool readOnly) { mReadOnly = readOnly; }
	bool isReadOnly() const { return mReadOnly; }

	using MetadataMap = std::map<std::string, std::any>;
	void addMetadata(const MetadataMap& metadata);
	void addMetadata(const std::string& name, const std::any& value);
	std::any getMetadata(const std::string& name) const;

protected:
	bool mReadOnly = false;
	const TypePtr mType;

private:
	std::string mName;
	std::map<std::string, std::any> mMetadata;
};

template <typename T>
std::optional<T> getOptionalValue(const std::any& v)
{
	if (!v.has_value())
	{
		return std::nullopt;
	}

    try
    {
        return std::any_cast<T>(v);
    }
    catch (const std::bad_any_cast&)
	{
		return std::nullopt;
	}
}

template <typename TypeT, typename MemberT>
class MemberProperty : public Property
{
public:
	MemberProperty(TypeRegistry* typeRegistry, const std::string& name, const TypePtr& type, MemberT TypeT::*member) :
		Property(name, type),
		mTypeRegistry(typeRegistry),
		mMember(member)
	{
		assert(mTypeRegistry);
	}

	~MemberProperty() override = default;

	bool setValue(Instance& obj, const Instance& value) override
	{
		if (mReadOnly)
		{
			return false;
		}

		obj.getObject<TypeT>()->*mMember = *value.getObject<MemberT>();
		return true;
	}

	Instance getValue(const Instance& obj) const override
	{
		const TypeT* objT = obj.getObject<TypeT>();
		return createNonOwningInstance(mTypeRegistry, const_cast<MemberT*>(&(objT->*mMember)));
	}

private:
	TypeRegistry* mTypeRegistry;
	MemberT TypeT::*mMember;
};

template <typename TypeT, typename GetterMemberT, typename SetterMemberT>
class GetterSetterMethodsProperty : public Property
{
public:
	GetterSetterMethodsProperty(TypeRegistry* typeRegistry, const std::string& name, const TypePtr& type, GetterMemberT (TypeT::*getter)() const, void (TypeT::*setter)(SetterMemberT)) :
		Property(name, type),
		mTypeRegistry(typeRegistry),
		mGetter(getter),
		mSetter(setter)
	{
		assert(mTypeRegistry);
	}

	~GetterSetterMethodsProperty() override = default;

	bool setValue(Instance& obj, const Instance& value) override
	{
		using RawMemberT = typename std::remove_cv<typename std::remove_reference<GetterMemberT>::type>::type;
		(obj.getObject<TypeT>()->*mSetter)(*value.getObject<RawMemberT>());
		return true;
	}

	Instance getValue(const Instance& obj) const override
	{
		return createOwningInstance(mTypeRegistry, (obj.getObject<TypeT>()->*mGetter)());
	}

private:
	TypeRegistry* mTypeRegistry;
	GetterMemberT (TypeT::*mGetter)() const;
	void (TypeT::*mSetter)(SetterMemberT);
};

template <typename TypeT, typename MemberT>
class GetterMethodsProperty : public Property
{
public:
	GetterMethodsProperty(TypeRegistry* typeRegistry, const std::string& name, const TypePtr& type, MemberT (TypeT::*getter)() const) :
		Property(name, type),
		mTypeRegistry(typeRegistry),
		mGetter(getter)
	{
		assert(mTypeRegistry);
	}

	~GetterMethodsProperty() override = default;

	bool setValue(Instance& obj, const Instance& value) override
	{
		return false;
	}

	Instance getValue(const Instance& obj) const override
	{
		return createOwningInstance(mTypeRegistry, (obj.getObject<TypeT>()->*mGetter)());
	}

private:
	TypeRegistry* mTypeRegistry;
	MemberT (TypeT::*mGetter)() const;
};

using TypeDefinitionRegistrationHandler = std::function<void(TypeRegistry& registry)>;
using TypeDefinitionRegisterLater = std::function<void(TypeDefinitionRegistrationHandler)>;

template <typename TypeT>
class TypeBuilder
{
public:
	//! Since properties refer to other types, we need to register types before properties.
	//! We achieve this with `registerLater`, which delays registration of properties until
	//! types have been registered.
	TypeBuilder(TypePtr type, TypeDefinitionRegisterLater registerLater) :
		mType(std::move(type)),
		mRegisterLater(std::move(registerLater))
	{}

	template <typename MemberT>
	TypeBuilder<TypeT>& property(const std::string& name, MemberT TypeT::*member, Property::MetadataMap metadata = {})
	{
		mRegisterLater([name = std::move(name), member = std::move(member), metadata = std::move(metadata), type = mType] (TypeRegistry& registry) {
			auto p = std::make_shared<MemberProperty<TypeT, MemberT>>(&registry, name, registry.getOrCreateType<MemberT>(), member);
			p->addMetadata(std::move(metadata));
			type->addProperty(p);
		});
		return *this;
	}

	template <typename MemberT>
	TypeBuilder<TypeT>& propertyReadOnly(const std::string& name, MemberT TypeT::*member, Property::MetadataMap metadata = {})
	{
		mRegisterLater([name = std::move(name), member = std::move(member), metadata = std::move(metadata), type = mType] (TypeRegistry& registry) {
			auto p = std::make_shared<MemberProperty<TypeT, MemberT>>(&registry, name, registry.getOrCreateType<MemberT>(), member);
			p->addMetadata(std::move(metadata));
			p->setReadOnly(true);
			type->addProperty(p);
		});
		return *this;
	}

	template <typename GetterMemberT, typename SetterMemberT>
	TypeBuilder<TypeT>& property(const std::string& name, GetterMemberT (TypeT::*getter)() const, void (TypeT::*setter)(SetterMemberT), Property::MetadataMap metadata = {})
	{
		mRegisterLater([name = std::move(name), getter = std::move(getter), setter = std::move(setter), metadata = std::move(metadata), type = mType] (TypeRegistry& registry) {
			using RawGetterMemberT = typename std::remove_cv<typename std::remove_reference<GetterMemberT>::type>::type;
			using RawSetterMemberT = typename std::remove_cv<typename std::remove_reference<SetterMemberT>::type>::type;
			static_assert(std::is_same<RawGetterMemberT, RawSetterMemberT>::value, "Getter and setter must be of same type, ignoring const ref qualifiers");

			auto p = std::make_shared<GetterSetterMethodsProperty<TypeT, GetterMemberT, SetterMemberT>>(&registry, name, registry.getOrCreateType<RawGetterMemberT>(), getter, setter);
			p->addMetadata(std::move(metadata));
			type->addProperty(p);
		});
		return *this;
	}

	template <typename MemberT>
	TypeBuilder<TypeT>& propertyReadOnly(const std::string& name, MemberT (TypeT::*getter)() const, Property::MetadataMap metadata = {})
	{
		mRegisterLater([name = std::move(name), getter = std::move(getter), metadata = std::move(metadata), type = mType] (TypeRegistry& registry) {
			auto p = std::make_shared<GetterMethodsProperty<TypeT, MemberT>>(&registry, name, registry.getOrCreateType<MemberT>(), getter);
			p->addMetadata(std::move(metadata));
			p->setReadOnly(true);
			type->addProperty(p);
		});
		return *this;
	}

	template <typename SuperT>
	TypeBuilder<TypeT>& superType()
	{
		mRegisterLater([type = mType] (TypeRegistry& registry) {
			TypeT* derived = reinterpret_cast<TypeT*>(reinterpret_cast<char*>(1 << 16));
			std::ptrdiff_t offset = reinterpret_cast<std::ptrdiff_t>(reinterpret_cast<char*>(static_cast<SuperT*>(derived))) - (1 << 16);
			type->addSuperType(registry.getOrCreateType<SuperT>(), offset);
		});
		return *this;
	}

private:
	TypePtr mType;
	TypeDefinitionRegisterLater mRegisterLater;
};

class TypeRegistryBuilder
{
public:
	TypeRegistryBuilder(TypeRegistry& registry, TypeDefinitionRegisterLater registerLater) :
		mRegistry(registry),
		mRegisterLater(std::move(registerLater))
	{}

	template <typename T>
	TypeBuilder<T> type(const std::string& name)
	{
		assert(mRegistry.getType<T>() == nullptr);
		std::type_index typeIndex = typeid(T);
		auto newType = std::make_shared<Type>(name, typeIndex);
		mRegistry.addType(newType);
		auto typeBuilder = TypeBuilder<T>(newType, mRegisterLater);
		return typeBuilder;
	}

private:
	TypeRegistry& mRegistry;
	TypeDefinitionRegisterLater mRegisterLater;
};

using RegistrationHandler = std::function<void(TypeRegistryBuilder&)>;

void addStaticRegistrationHandler(RegistrationHandler handler);
void addStaticallyRegisteredTypes(TypeRegistry& registry);

#define SKYBOLT_REFLECT_BEGIN(cls)                                     \
	const int auto_register__##cls = []() {                            \
		skybolt::refl::addStaticRegistrationHandler([] (skybolt::refl::TypeRegistryBuilder& registry)

#define SKYBOLT_REFLECT_END    \
		);                     \
		return 0;              \
	}();

#define SKYBOLT_REFLECT_EXTERN(cls)                                   \
	extern const int auto_register__##cls;                            \
	static const int use_auto_register__##cls = []() {                \
		return auto_register__##cls;                                  \
	}();

} // namespace skybolt::refl