/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#include <catch2/catch.hpp>
#include <SkyboltCommon/TypedItemContainer.h>

using namespace skybolt;

struct Base
{
	virtual ~Base() {}
};

struct DerivedA : Base {};
struct DerivedB : Base {};

struct MultiTypeBase
{
	virtual ~MultiTypeBase() {}
};

struct SomeOtherType {};

struct MultiTypeDerived : MultiTypeBase, SomeOtherType {};

namespace skybolt {

template<>
std::vector<std::type_index> getExposedTypes<MultiTypeBase>(const MultiTypeBase& type)
{
	return { typeid(MultiTypeDerived), typeid(MultiTypeBase), typeid(SomeOtherType) };
}

}

TEST_CASE("TypedItemContainer add and remove items of different types")
{
	TypedItemContainer<Base> c;

	auto itemA = std::make_shared<DerivedA>();
	c.addItem(itemA);

	auto itemB = std::make_shared<DerivedB>();
	c.addItem(itemB);

	{
		auto items = c.getAllItems();
		REQUIRE(items.size() == 2);
		CHECK(items[0] == itemA);
		CHECK(items[1] == itemB);
	}

	{
		auto items = c.getItemsOfType<DerivedA>();
		REQUIRE(items.size() == 1);
		CHECK(items[0] == itemA);
	}

	CHECK(c.getFirstItemOfType<DerivedA>() == itemA);

	c.removeItem(itemA);
	CHECK(c.getFirstItemOfType<DerivedA>() == nullptr);
}

TEST_CASE("TypedItemContainer add and remove items of same type")
{
	TypedItemContainer<Base> c;

	auto itemA = std::make_shared<DerivedA>();
	c.addItem(itemA);

	auto itemB = std::make_shared<DerivedA>();
	c.addItem(itemB);

	{
		auto items = c.getAllItems();
		REQUIRE(items.size() == 2);
		CHECK(items[0] == itemA);
		CHECK(items[1] == itemB);
	}

	{
		auto items = c.getItemsOfType<DerivedA>();
		REQUIRE(items.size() == 2);
		CHECK(items[0] == itemA);
		CHECK(items[1] == itemB);
	}

	c.removeItem(itemA);
	CHECK(c.getFirstItemOfType<DerivedA>() == itemB);
}

TEST_CASE("TypedItemContainer add and remove item with multiple exposed types")
{
	TypedItemContainer<MultiTypeBase> c;

	auto item = std::make_shared<MultiTypeDerived>();
	c.addItem(item);

	CHECK(c.getAllItems().size() == 1);
	CHECK(c.getFirstItemOfType<MultiTypeDerived>() == item);
	CHECK(c.getFirstItemOfType<MultiTypeBase>() == item);
	CHECK(c.getFirstItemOfType<SomeOtherType>() == item);

	c.removeItem(item);
	CHECK(c.getFirstItemOfType<MultiTypeDerived>() == nullptr);
	CHECK(c.getFirstItemOfType<MultiTypeBase>() == nullptr);
	CHECK(c.getAllItems().empty());
}
