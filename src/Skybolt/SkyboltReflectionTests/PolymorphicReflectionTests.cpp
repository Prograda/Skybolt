/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include <catch2/catch.hpp>
#include <SkyboltReflection/Reflection.h>

using namespace skybolt::refl;

struct BaseA
{
	virtual ~BaseA() = default;

	int intA = 0;
};

struct BaseB
{
	virtual ~BaseB() = default;

	int intB = 0;
};

struct Derived : public BaseA, BaseB
{
	~Derived() override = default;

	int intC = 0;
};

SKYBOLT_REFLECT_BEGIN(BaseA)
{
	registry.type<BaseA>("BaseA")
		.property("intA", &BaseA::intA);
}
SKYBOLT_REFLECT_END

SKYBOLT_REFLECT_BEGIN(BaseB)
{
	registry.type<BaseB>("BaseB")
		.property("intB", &BaseB::intB);
}
SKYBOLT_REFLECT_END

SKYBOLT_REFLECT_BEGIN(Derived)
{
	registry.type<Derived>("Derived")
		.superType<BaseA>()
		.superType<BaseB>()
		.property("intC", &Derived::intC);
}
SKYBOLT_REFLECT_END

TEST_CASE("Derived type has super type properties")
{
	TypeRegistry registry;

	auto type = registry.getTypeByName("Derived");
	REQUIRE(type);
	CHECK(type->isDerivedFrom<BaseA>());
	CHECK(type->isDerivedFrom<BaseB>());
	CHECK(type->getProperty("intA"));
	CHECK(type->getProperty("intB"));
	CHECK(type->getProperty("intC"));
}

TEST_CASE("Access properties of type with multiple super classes")
{
	TypeRegistry registry;

	auto type = registry.getTypeByName("Derived");
	REQUIRE(type);
	auto derivedClassProperty = type->getProperty("intC");
	REQUIRE(derivedClassProperty);
	auto firstBaseClassProperty = type->getProperty("intA");
	REQUIRE(firstBaseClassProperty);
	auto secondBaseClassProperty = type->getProperty("intB");
	REQUIRE(secondBaseClassProperty);
	
	Derived derivedObj;

	SECTION("Set derived class value on instance created from derived class")
	{
		auto instance = createNonOwningInstance(&registry, &derivedObj);
		derivedClassProperty->setValue(instance, createOwningInstance(&registry, 1));
		CHECK(derivedObj.intC == 1);
	}

	SECTION("Set derived class value on instance created from second base class")
	{
		BaseB* baseObj = &derivedObj;
		auto instance = createNonOwningInstance(&registry, baseObj);
		derivedClassProperty->setValue(instance, createOwningInstance(&registry, 2));
		CHECK(derivedObj.intC == 2);
	}

	SECTION("Set derived class value on instance created from second base class")
	{
		BaseB* baseObj = &derivedObj;
		auto instance = createNonOwningInstance(&registry, baseObj);
		derivedClassProperty->setValue(instance, createOwningInstance(&registry, 3));
		CHECK(derivedObj.intC == 3);
	}

	SECTION("Set first base class value on instance created from second base class")
	{
		BaseB* baseObj = &derivedObj;
		auto instance = createNonOwningInstance(&registry, baseObj);
		firstBaseClassProperty->setValue(instance, createOwningInstance(&registry, 1));
		CHECK(derivedObj.intA == 1);
	}

	SECTION("Set second base class value on instance created from first base class")
	{
		BaseA* baseObj = &derivedObj;
		auto instance = createNonOwningInstance(&registry, baseObj);
		secondBaseClassProperty->setValue(instance, createOwningInstance(&registry, 2));
		CHECK(derivedObj.intB == 2);
	}

	SECTION("Set second base class value on instance created from derived class")
	{
		auto instance = createNonOwningInstance(&registry, &derivedObj);
		secondBaseClassProperty->setValue(instance, createOwningInstance(&registry, 3));
		CHECK(derivedObj.intB == 3);
	}
}
