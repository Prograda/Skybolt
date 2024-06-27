/* Copyright 2012-2020 Matthew Reid
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

	TestObject object;
	ReflInstanceGetter instanceGetter = [&] { return refl::createNonOwningInstance(&typeRegistry, &object); };
	refl::PropertyPtr reflProperty = typeRegistry.getTypeRequired<TestObject>()->getProperty("intProperty");

	std::optional<QtPropertyUpdaterApplier> qtProperty = reflPropertyToQt(typeRegistry, instanceGetter, reflProperty);
	REQUIRE(qtProperty);
	auto property = qtProperty->property.get();
	REQUIRE(property);

	// Set model property and check that its value is reflected to Qt
	object.intProperty = 123;
	qtProperty->updater(*property);
	CHECK(property->value == 123);

	// Set Qt property and check that its value is reflected to model
	property->setValue(456);
	qtProperty->applier(*qtProperty->property);
	CHECK(object.intProperty == 456);
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

	TestObjectContainingOptional object;
	ReflInstanceGetter instanceGetter = [&] { return refl::createNonOwningInstance(&typeRegistry, &object); };
	refl::PropertyPtr reflProperty = typeRegistry.getTypeRequired<TestObjectContainingOptional>()->getProperty("value");

	std::optional<QtPropertyUpdaterApplier> qtProperty = reflPropertyToQt(typeRegistry, instanceGetter, reflProperty);
	REQUIRE(qtProperty);
	auto property = qtProperty->property.get();
	REQUIRE(property);

	REQUIRE(property->value.userType() == qMetaTypeId<OptionalProperty>());

	auto optionalProperty = property->value.value<OptionalProperty>();
	REQUIRE(optionalProperty.property);
	CHECK(!optionalProperty.present);

	// Set model property and check that its value is reflected to Qt
	object.value = 123.0;
	qtProperty->updater(*property);
	optionalProperty = property->value.value<OptionalProperty>();
	CHECK(optionalProperty.property->value == 123.0);
	CHECK(optionalProperty.present);

	// Set Qt property and check that its value is reflected to model
	optionalProperty.property->setValue(456.0);
	qtProperty->applier(*property);
	CHECK(object.value == 456.0);

	optionalProperty.present = false;
	property->setValue(QVariant::fromValue(optionalProperty));
	qtProperty->applier(*property);
	CHECK(!object.value.has_value());
}
