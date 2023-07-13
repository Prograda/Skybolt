/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#include <SkyboltSim/Serialization/Serialization.h>
#include <catch2/catch.hpp>

#include <assert.h>

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
	std::map<std::string, double> doubleMapProperty;
	TestNestedObject nestedObjectProperty;
	std::shared_ptr<TestNestedObject> nestedObjectSharedPtrProperty = std::make_shared<TestNestedObject>();
	std::map<std::string, std::shared_ptr<TestNestedObject>> sharedPtrMapProperty = { {"A", std::make_shared<TestNestedObject>()} };
	std::vector<std::shared_ptr<TestNestedObject>> sharedPtrVectorProperty = { std::make_shared<TestNestedObject>(-1), std::make_shared<TestNestedObject>(-1)};
};

SKYBOLT_REFLECT_INLINE(TestNestedObject)
{
	rttr::registration::class_<TestNestedObject>("TestNestedObject")
		.property("intProperty", &TestNestedObject::intProperty);
}

SKYBOLT_REFLECT_INLINE(TestObject)
{
	rttr::registration::class_<TestObject>("TestObject")
		.property("intProperty", &TestObject::intProperty)
		.property("doubleMapProperty", &TestObject::doubleMapProperty)
		.property("nestedObjectProperty", &TestObject::nestedObjectProperty)
		.property("nestedObjectSharedPtrProperty", &TestObject::nestedObjectSharedPtrProperty)
		.property("sharedPtrMapProperty", &TestObject::sharedPtrMapProperty)
		.property("sharedPtrVectorProperty", &TestObject::sharedPtrVectorProperty);
}

TEST_CASE("Read and write to JSON")
{
	// Write
	TestObject originalObject;
	originalObject.intProperty = 2;
	originalObject.doubleMapProperty = { {"A", 1}, {"B", 2} };
	originalObject.nestedObjectProperty.intProperty = 3;
	originalObject.nestedObjectSharedPtrProperty->intProperty = 4;
	originalObject.sharedPtrMapProperty["A"]->intProperty = 6;
	originalObject.sharedPtrVectorProperty = { std::make_shared<TestNestedObject>(1), std::make_shared<TestNestedObject>(2) };
	nlohmann::json json = writeReflectedObject(originalObject);

	// Read
	TestObject readObject;
	readReflectedObject(rttr::instance(readObject), json);

	CHECK(readObject.intProperty == originalObject.intProperty);
	CHECK(readObject.doubleMapProperty == originalObject.doubleMapProperty);
	CHECK(readObject.nestedObjectProperty.intProperty == originalObject.nestedObjectProperty.intProperty);
	REQUIRE(readObject.nestedObjectSharedPtrProperty);
	CHECK(readObject.nestedObjectSharedPtrProperty->intProperty == originalObject.nestedObjectSharedPtrProperty->intProperty);
	REQUIRE(readObject.sharedPtrMapProperty.size() == 1);
	CHECK(readObject.sharedPtrMapProperty.begin()->first == originalObject.sharedPtrMapProperty.begin()->first);
	REQUIRE(readObject.sharedPtrMapProperty.begin()->second);
	CHECK(readObject.sharedPtrMapProperty.begin()->second->intProperty == originalObject.sharedPtrMapProperty.begin()->second->intProperty);
	REQUIRE(readObject.sharedPtrVectorProperty.size() == 2);
	CHECK(readObject.sharedPtrVectorProperty[0]->intProperty == 1);
	CHECK(readObject.sharedPtrVectorProperty[1]->intProperty == 2);
}

struct TestBaseObject
{
	SKYBOLT_ENABLE_POLYMORPHIC_REFLECTION()
public:
	virtual ~TestBaseObject() = default;
};

struct TestDerivedObject : public TestBaseObject
{
	SKYBOLT_ENABLE_POLYMORPHIC_REFLECTION(TestBaseObject)
public:
	TestDerivedObject() {}
	TestDerivedObject(float floatProperty) : floatProperty(floatProperty) {}
	~TestDerivedObject() override = default;

	float floatProperty;
};

SKYBOLT_REFLECT_INLINE(TestDerivedObject)
{
	rttr::registration::class_<TestDerivedObject>("TestDerivedObject")
		.property("floatProperty", &TestDerivedObject::floatProperty);
}

TEST_CASE("Read and write polymorphic type to JSON")
{
	// Write
	TestDerivedObject derivedObject;
	derivedObject.floatProperty = 123;

	const TestBaseObject& baseObject = derivedObject;
	nlohmann::json json = writeReflectedObject(baseObject); // test writing the base object reference

	// Read
	TestDerivedObject readObject;
	readReflectedObject(rttr::instance(readObject), json);

	CHECK(readObject.floatProperty == derivedObject.floatProperty);
}

struct TestObjectWithSerializationMethods : public ExplicitSerialization
{
	SKYBOLT_ENABLE_POLYMORPHIC_REFLECTION(ExplicitSerialization);
public:
	nlohmann::json toJson() const
	{
		return data;
	};

	void fromJson(const nlohmann::json& j)
	{
		data = j.get<int>();
	}

	int data = 0;
};

TEST_CASE("Use explicit to/from json methods if an object provides them")
{
	// Write
	TestObjectWithSerializationMethods writeObject;
	writeObject.data = 123;
	nlohmann::json json = writeReflectedObject(writeObject);

	// Read
	TestObjectWithSerializationMethods readObject;
	readReflectedObject(rttr::instance(readObject), json);
	CHECK(readObject.data == 123);
}