/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#include <SkyboltSim/Serialization/Serialization.h>
#include <catch2/catch.hpp>

#include <assert.h>
#include <optional>

using namespace skybolt;
using namespace skybolt::sim;

struct TestNestedObject
{
public:
	TestNestedObject() {}
	TestNestedObject(int intProperty) : intProperty(intProperty) {}

	int intProperty;
};

struct TestObject
{
	int intProperty;
	TestNestedObject nestedObjectProperty;
	std::optional<int> optionalIntProperty1;
	std::optional<int> optionalIntProperty2;
};

SKYBOLT_REFLECT_BEGIN(TestNestedObject)
{
	registry.type<TestNestedObject>("TestNestedObject")
		.property("intProperty", &TestNestedObject::intProperty);
}
SKYBOLT_REFLECT_END

SKYBOLT_REFLECT_BEGIN(TestObject)
{
	registry.type<TestObject>("TestObject")
		.property("intProperty", &TestObject::intProperty)
		.property("nestedObjectProperty", &TestObject::nestedObjectProperty)
		.property("optionalIntProperty1", &TestObject::optionalIntProperty1)
		.property("optionalIntProperty2", &TestObject::optionalIntProperty2);
}
SKYBOLT_REFLECT_END

TEST_CASE("Read and write to JSON")
{
	refl::TypeRegistry registry;

	// Write
	TestObject originalObject;
	originalObject.intProperty = 2;
	originalObject.nestedObjectProperty.intProperty = 3;
	// Leave originalObject.optionalIntProperty1 unset
	originalObject.optionalIntProperty2 = 123;
	nlohmann::json json = writeReflectedObject(registry, refl::createNonOwningInstance(&registry, &originalObject));

	// Read
	TestObject readObject;
	auto instance = refl::createNonOwningInstance(&registry, &readObject);
	readReflectedObject(registry, instance, json);

	CHECK(readObject.intProperty == originalObject.intProperty);
	CHECK(readObject.nestedObjectProperty.intProperty == originalObject.nestedObjectProperty.intProperty);
	CHECK(!readObject.optionalIntProperty1);
	REQUIRE(readObject.optionalIntProperty2);
	CHECK(readObject.optionalIntProperty2 == 123);
}

struct TestBaseObject
{
public:
	virtual ~TestBaseObject() = default;
};

struct TestDerivedObject : public TestBaseObject
{
public:
	TestDerivedObject() {}
	TestDerivedObject(float floatProperty) : floatProperty(floatProperty) {}
	~TestDerivedObject() override = default;

	float floatProperty;
};

SKYBOLT_REFLECT_BEGIN(TestDerivedObject)
{
	registry.type<TestDerivedObject>("TestDerivedObject")
		.superType<TestBaseObject>()
		.property("floatProperty", &TestDerivedObject::floatProperty);
}
SKYBOLT_REFLECT_END

TEST_CASE("Read and write polymorphic type to JSON")
{
	refl::TypeRegistry registry;

	// Write
	TestDerivedObject derivedObject;
	derivedObject.floatProperty = 123;

	TestBaseObject& baseObject = derivedObject;
	nlohmann::json json = writeReflectedObject(registry, refl::createNonOwningInstance(&registry, &baseObject)); // test writing the base object reference

	// Read
	TestDerivedObject readObject;
	auto instance = refl::createNonOwningInstance(&registry, &readObject);
	readReflectedObject(registry, instance, json);

	CHECK(readObject.floatProperty == derivedObject.floatProperty);
}

struct TestObjectWithSerializationMethods : public ExplicitSerialization
{
public:
	nlohmann::json toJson(refl::TypeRegistry& typeRegistry) const
	{
		return data;
	};

	void fromJson(refl::TypeRegistry& typeRegistry, const nlohmann::json& j)
	{
		data = j.get<int>();
	}

	int data = 0;
};

SKYBOLT_REFLECT_BEGIN(TestObjectWithSerializationMethods)
{
	registry.type<TestObjectWithSerializationMethods>("TestObjectWithSerializationMethods")
		.superType<ExplicitSerialization>();
}
SKYBOLT_REFLECT_END

TEST_CASE("Use explicit to/from json methods if an object provides them")
{
	refl::TypeRegistry registry;

	// Write
	TestObjectWithSerializationMethods writeObject;
	writeObject.data = 123;
	nlohmann::json json = writeReflectedObject(registry, refl::createNonOwningInstance(&registry, &writeObject));

	// Read
	TestObjectWithSerializationMethods readObject;
	auto instance = refl::createNonOwningInstance(&registry, &readObject);
	readReflectedObject(registry, instance, json);
	CHECK(readObject.data == 123);
}