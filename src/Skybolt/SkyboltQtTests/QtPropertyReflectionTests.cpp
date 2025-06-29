/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include <SkyboltReflection/Reflection.h>
#include <SkyboltQt/Property/QtMetaTypes.h>
#include <SkyboltQt/Property/QtPropertyReflection.h>

namespace refl = skybolt::refl;

struct TestObject
{
	int intProperty;
};

SKYBOLT_REFLECT_BEGIN(TestObject)
{
	registry.type<TestObject>("TestObject")
		.property("intProperty", &TestObject::intProperty);
}
SKYBOLT_REFLECT_END

TEST_CASE("Reflect basic property to Qt")
{
	refl::TypeRegistry typeRegistry;

	// Create model property
	TestObject object;
	ReflInstanceGetter instanceGetter = [&] { return refl::createNonOwningInstance(typeRegistry, &object); };
	refl::PropertyPtr reflProperty = typeRegistry.getTypeRequired<TestObject>()->getProperty("intProperty");

	// Create reflected UI property
	std::optional<QtPropertyUpdaterApplier> qtProperty = reflPropertyToQt(typeRegistry, instanceGetter, reflProperty);
	REQUIRE(qtProperty);
	auto property = qtProperty->property.get();
	REQUIRE(property);

	// Listen to property value change events
	int valueChangeCount = 0;
	QObject::connect(property, &QtProperty::valueChanged, [&]() { valueChangeCount++; });

	// Set model property and check that its value is reflected to Qt
	object.intProperty = 123;
	qtProperty->updater(*property);
	CHECK(property->value() == 123);
	CHECK(valueChangeCount == 1);

	// Set Qt property and check that its value is reflected to model
	property->setValue(456);
	qtProperty->applier(*qtProperty->property);
	CHECK(object.intProperty == 456);
	CHECK(valueChangeCount == 2);
}

struct TestObjectContainingOptional
{
	std::optional<double> value;
};

SKYBOLT_REFLECT_BEGIN(TestObjectContainingOptional)
{
	registry.type<TestObjectContainingOptional>("TestObjectContainingOptional")
		.property("value", &TestObjectContainingOptional::value);
}
SKYBOLT_REFLECT_END

TEST_CASE("Reflect optional property to Qt")
{
	refl::TypeRegistry typeRegistry;

	// Create model property
	TestObjectContainingOptional object;
	ReflInstanceGetter instanceGetter = [&] { return refl::createNonOwningInstance(typeRegistry, &object); };
	refl::PropertyPtr reflProperty = typeRegistry.getTypeRequired<TestObjectContainingOptional>()->getProperty("value");
	REQUIRE(reflProperty);

	// Create reflected UI property
	std::optional<QtPropertyUpdaterApplier> propertyUpdater = reflPropertyToQt(typeRegistry, instanceGetter, reflProperty);
	REQUIRE(propertyUpdater);
	auto property = propertyUpdater->property.get();
	REQUIRE(property);
	REQUIRE(property->value().userType() == qMetaTypeId<OptionalProperty>());

	// Listen to property value change events
	int valueChangeCount = 0;
	QObject::connect(property, &QtProperty::valueChanged, [&]() { valueChangeCount++; });

	// Check that property value is initially absent
	auto optionalProperty = property->value().value<OptionalProperty>();
	REQUIRE(optionalProperty.property);
	CHECK(!optionalProperty.present);
	CHECK(valueChangeCount == 0);

	// Set model property and check that its value is reflected to Qt
	object.value = 123.0;
	propertyUpdater->updater(*property);
	optionalProperty = property->value().value<OptionalProperty>();
	CHECK(optionalProperty.property->value() == 123.0);
	CHECK(optionalProperty.present);
	CHECK(valueChangeCount == 1);

	// Set model property to absent and check that absent state is reflected to Qt,
	// but the Qt child property value is unchanged so that the last entry is preserved
	// for next time the optional switched to present.
	object.value = std::nullopt;
	propertyUpdater->updater(*property);
	optionalProperty = property->value().value<OptionalProperty>();
	CHECK(optionalProperty.property->value() == 123.0);
	CHECK(!optionalProperty.present);
	CHECK(valueChangeCount == 2);

	// Set Qt property value and check that its value is reflected to model
	optionalProperty.present = true;
	optionalProperty.property->setValue(456.0);
	property->setValue(QVariant::fromValue(optionalProperty));
	propertyUpdater->applier(*property);
	CHECK(object.value == 456.0);
	CHECK(valueChangeCount == 4); // two change events - one for child and one for parent

	// Set Qt property to be absent and check that it's reflected in model
	optionalProperty.present = false;
	property->setValue(QVariant::fromValue(optionalProperty));
	propertyUpdater->applier(*property);
	CHECK(!object.value.has_value());
	CHECK(valueChangeCount == 5);
}

struct TestObjectContainingVector
{
	std::vector<int> values;
};

SKYBOLT_REFLECT_BEGIN(TestObjectContainingVector)
{
	registry.type<TestObjectContainingVector>("TestObjectContainingVector")
		.property("values", &TestObjectContainingVector::values);
}
SKYBOLT_REFLECT_END

TEST_CASE("Reflect vector property to Qt")
{
	refl::TypeRegistry typeRegistry;

	// Create model property
	TestObjectContainingVector object;
	ReflInstanceGetter instanceGetter = [&] { return refl::createNonOwningInstance(typeRegistry, &object); };
	refl::PropertyPtr reflProperty = typeRegistry.getTypeRequired<TestObjectContainingVector>()->getProperty("values");
	REQUIRE(reflProperty);

	// Create reflected UI property
	std::optional<QtPropertyUpdaterApplier> propertyUpdater = reflPropertyToQt(typeRegistry, instanceGetter, reflProperty);
	REQUIRE(propertyUpdater);
	auto property = propertyUpdater->property.get();
	REQUIRE(property);
	REQUIRE(property->value().userType() == qMetaTypeId<PropertyVector>());

	// Listen to property value change events
	int valueChangeCount = 0;
	QObject::connect(property, &QtProperty::valueChanged, [&]() { valueChangeCount++; });

	// Assert that vector is initially empty
	auto propertyVector = property->value().value<PropertyVector>();
	CHECK(propertyVector.items.empty());
	CHECK(valueChangeCount == 0);

	// Set model property and check that its value is reflected to Qt
	object.values = {1, 2};
	propertyUpdater->updater(*property);
	propertyVector = property->value().value<PropertyVector>();
	REQUIRE(propertyVector.items.size() == 2);
	CHECK(propertyVector.items.front()->value().toInt() == 1);
	CHECK(propertyVector.items.back()->value().toInt() == 2);
	CHECK(valueChangeCount == 1);

	// Modify vector property and check that its value is reflected to model
	propertyVector.items.resize(1);
	property->setValue(QVariant::fromValue(propertyVector));
	propertyUpdater->applier(*property);
	CHECK(object.values == std::vector<int>{1});
	CHECK(valueChangeCount == 2);

	// Modify vector item property and check that its value is reflected to model
	propertyVector.items.front()->setValue(2);
	propertyUpdater->applier(*property);
	CHECK(object.values == std::vector<int>{2});
	CHECK(valueChangeCount == 3);
}