/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include <SkyboltReflection/Reflection.h>

using namespace skybolt::refl;

struct TestClass
{
	virtual ~TestClass() = default;

	int intProperty = 0;
	int getterSetterMethodProperty = 0;
	int getterSetterFunctionProperty = 0;

	void setGetterSetterMethodProperty(int v) { getterSetterMethodProperty = v; }
	int getGetterSetterMethodProperty() const { return getterSetterMethodProperty; }
};

SKYBOLT_REFLECT_BEGIN(TestClass)
{
	registry.type<TestClass>("TestClass")
		.property("intProperty", &TestClass::intProperty)
		.property("intPropertyWithMetadata", &TestClass::intProperty, {{ "key1", std::string("value1") }})
		.property("getterSetterMethodProperty", &TestClass::getGetterSetterMethodProperty, &TestClass::setGetterSetterMethodProperty)
		.propertyFn<int, int>("getterSetterFunctionProperty", [] (const TestClass& obj) { return obj.getterSetterFunctionProperty; }, [] (TestClass& obj, int value) { obj.getterSetterFunctionProperty = value; });
}
SKYBOLT_REFLECT_END

TEST_CASE("Get registered class by type and name")
{
	TypeRegistry registry;
	registry.addType(std::make_shared<Type>("hello", typeid(TestClass)));
	CHECK(registry.getType<TestClass>() != nullptr);
	CHECK(registry.getTypeByName("hello") != nullptr);
}

TEST_CASE("Properties backed by class members")
{
	TypeRegistry registry;

	auto type = registry.getTypeByName("TestClass");
	REQUIRE(type);
	auto property = type->getProperty("intProperty");
	REQUIRE(property);
	
	TestClass obj;
	auto instance = createNonOwningInstance(&registry, &obj);
	property->setValue(instance, createOwningInstance(&registry, 123));
	CHECK(obj.intProperty == 123);
	
	obj.intProperty = 456;
	CHECK(*property->getValue(instance).getObject<int>() == 456);
}

TEST_CASE("Properties backed by getter and setter methods")
{
	TypeRegistry registry;

	auto type = registry.getTypeByName("TestClass");
	REQUIRE(type);
	auto property = type->getProperty("getterSetterMethodProperty");
	REQUIRE(property);
	
	TestClass obj;
	auto instance = createNonOwningInstance(&registry, &obj);
	property->setValue(instance, createOwningInstance(&registry, 123));
	CHECK(obj.getterSetterMethodProperty == 123);
	
	obj.getterSetterMethodProperty = 456;
	CHECK(*property->getValue(instance).getObject<int>() == 456);
}

TEST_CASE("Properties backed by getter and setter functions")
{
	TypeRegistry registry;

	auto type = registry.getTypeByName("TestClass");
	REQUIRE(type);

	auto property = type->getProperty("getterSetterFunctionProperty");
	REQUIRE(property);
	
	TestClass obj;
	auto instance = createNonOwningInstance(&registry, &obj);
	property->setValue(instance, createOwningInstance(&registry, 123));
	CHECK(obj.getterSetterFunctionProperty == 123);
	
	obj.getterSetterFunctionProperty = 456;
	CHECK(*property->getValue(instance).getObject<int>() == 456);
}

TEST_CASE("Get property metadata")
{
	TypeRegistry registry;

	auto type = registry.getTypeByName("TestClass");
	REQUIRE(type);
	auto property = type->getProperty("intPropertyWithMetadata");
	REQUIRE(property);
	
	CHECK(std::any_cast<std::string>(property->getMetadata("key1")) == "value1");
}

struct TestValue {};
struct TypeReferringToAnotherType
{
	TestValue value;
};

SKYBOLT_REFLECT_BEGIN(TypeReferringToAnotherType)
{
	registry.type<TypeReferringToAnotherType>("TypeReferringToAnotherType")
		.property("value", &TypeReferringToAnotherType::value);
}
SKYBOLT_REFLECT_END

// Register TestValue after the type that refers to it so that we can test that registration order does not matter
SKYBOLT_REFLECT_BEGIN(TestValue)
{
	registry.type<TestValue>("TestValueCustomName");
}
SKYBOLT_REFLECT_END

TEST_CASE("Type that type can be registered after the type that refers to it")
{
	// This tests that delayed registration of type definitions works correctly.
	// All the explicitly reflected types must be registered before type properties which may
	// refer to other types.
	TypeRegistry registry;

	auto type = registry.getTypeByName("TypeReferringToAnotherType");
	REQUIRE(type);
	auto property = type->getProperty("value");
	REQUIRE(property);
	
	TypeReferringToAnotherType object;
	Instance value = property->getValue(createNonOwningInstance(&registry, &object));
	CHECK(value.getType()->getName() == "TestValueCustomName");
}