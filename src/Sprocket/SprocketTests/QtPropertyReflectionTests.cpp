/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include <SkyboltSim/Reflection.h>
#include <Sprocket/Property/QtPropertyReflection.h>

struct TestObject
{
	int intProperty;
};

SKYBOLT_REFLECT_INLINE(TestObject)
{
	rttr::registration::class_<TestObject>("TestObject")
		.property("intProperty", &TestObject::intProperty);
}

TEST_CASE("Reflect property to Qt")
{
	TestObject object;
	RttrInstanceGetter instanceGetter = [&] { return rttr::instance(object); };
	rttr::property property = rttr::type::get(object).get_property("intProperty");

	std::optional<QtPropertyUpdaterApplier> qtProperty = rttrPropertyToQt(instanceGetter, property);
	REQUIRE(qtProperty);
	auto variantProperty = dynamic_cast<VariantProperty*>(qtProperty->property.get());
	REQUIRE(variantProperty);

	// Set model property and check that its value is reflected to Qt
	object.intProperty = 123;
	qtProperty->updater(*qtProperty->property);
	CHECK(variantProperty->value == 123);

	// Set Qt property and check that its value is reflected to model
	variantProperty->setValue(456);
	qtProperty->applier(*qtProperty->property);
	CHECK(object.intProperty == 456);
}
