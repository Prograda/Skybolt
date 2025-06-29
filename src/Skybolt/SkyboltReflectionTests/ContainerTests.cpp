/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include <catch2/catch.hpp>
#include <SkyboltReflection/Reflection.h>

using namespace skybolt::refl;

TEST_CASE("Type has expected container accessor class")
{
	TypeRegistry registry;

	SECTION("value")
	{
		auto accessor = registry.getOrCreateType<int>()->getContainerValueAccessor();
		CHECK(accessor == nullptr);
	}

	SECTION("std::optional")
	{
		auto accessor = registry.getOrCreateType<std::optional<int>>()->getContainerValueAccessor();
		auto optionalAccessor = dynamic_cast<StdOptionalValueAccessor*>(accessor.get());
		REQUIRE(optionalAccessor);

		CHECK(optionalAccessor->valueTypeName == registry.getOrCreateType<int>()->getName());
	}

	SECTION("std::vector")
	{
		auto accessor = registry.getOrCreateType<std::vector<int>>()->getContainerValueAccessor();
		auto vectorAccessor = dynamic_cast<StdVectorValueAccessor*>(accessor.get());
		REQUIRE(vectorAccessor);

		CHECK(vectorAccessor->valueTypeName == registry.getOrCreateType<int>()->getName());
	}
}

TEST_CASE("std::optional accessor can read and write value")
{
	TypeRegistry registry;

	auto accessor = registry.getOrCreateType<std::optional<int>>()->getContainerValueAccessor();
	auto optionalAccessor = dynamic_cast<StdOptionalValueAccessor*>(accessor.get());
	REQUIRE(optionalAccessor);

	SECTION("Read empty optional")
	{
		Instance instance = createOwningInstance(registry, std::optional<int>());
		auto values = optionalAccessor->getValues(registry, instance);
		CHECK(values.empty());
	}

	SECTION("Read present optional")
	{
		Instance instance = createOwningInstance(registry, std::optional<int>(123));
		auto values = optionalAccessor->getValues(registry, instance);
		REQUIRE(values.size() == 1);
		CHECK(*values.front().getObject<int>() == 123);
	}

	SECTION("Write empty optional")
	{
		// Create with optional present
		Instance instance = createOwningInstance(registry, std::optional<int>(123));

		// Set null optional
		optionalAccessor->setValues(instance, {createOwningInstance(registry, std::optional<int>())});

		// Check that optional is null
		auto values = optionalAccessor->getValues(registry, instance);
		CHECK(values.empty());
	}

	SECTION("Write present optional")
	{
		Instance instance = createOwningInstance(registry, std::optional<int>(123));
		auto values = optionalAccessor->getValues(registry, instance);
		REQUIRE(values.size() == 1);
		CHECK(*values.front().getObject<int>() == 123);
	}
}

TEST_CASE("std::vector accessor can read and write values")
{
	TypeRegistry registry;

	auto accessor = registry.getOrCreateType<std::vector<int>>()->getContainerValueAccessor();
	auto vectorAccessor = dynamic_cast<StdVectorValueAccessor*>(accessor.get());
	REQUIRE(vectorAccessor);

	SECTION("Read empty vector")
	{
		Instance instance = createOwningInstance(registry, std::vector<int>());
		auto values = vectorAccessor->getValues(registry, instance);
		CHECK(values.empty());
	}

	SECTION("Read vector with values")
	{
		Instance instance = createOwningInstance(registry, std::vector<int>({1, 2}));
		auto values = vectorAccessor->getValues(registry, instance);
		REQUIRE(values.size() == 2);
		CHECK(*values.front().getObject<int>() == 1);
		CHECK(*values.back().getObject<int>() == 2);
	}

	SECTION("Write empty vector")
	{
		// Create with values in vector
		Instance instance = createOwningInstance(registry, std::vector<int>({1, 2}));

		// Clear vector
		vectorAccessor->setValues(instance, {});

		// Check that vector is empty
		auto values = vectorAccessor->getValues(registry, instance);
		CHECK(values.empty());
	}

	SECTION("Write vector with values")
	{
		// Create with empty vector
		Instance instance = createOwningInstance(registry, std::vector<int>());

		// Add value to vector
		vectorAccessor->setValues(instance, {createOwningInstance(registry, 1)});

		// Check that vector has values
		auto values = vectorAccessor->getValues(registry, instance);
		REQUIRE(values.size() == 1);
		CHECK(*values.front().getObject<int>() == 1);
	}
}
