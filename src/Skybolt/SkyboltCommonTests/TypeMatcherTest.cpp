/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#include <catch2/catch.hpp>
#include <SkyboltCommon/TypeMatcher.h>

using namespace skybolt;

struct Base
{
	virtual ~Base() = default;
};

struct DerivedA : Base {};
struct DerivedB : Base {};

TEST_CASE("TypeMatcher matches class types")
{
	bool callbackACalled = false;
	bool callbackBCalled = false;

	DerivedA derived;
	TypeMatcher(derived,
		[&] (const DerivedA& a) {
			callbackACalled = true;
		},
		[&] (const DerivedB& a) {
			callbackBCalled = true;
		}
	);
	CHECK(callbackACalled);
	CHECK(!callbackBCalled);
}
