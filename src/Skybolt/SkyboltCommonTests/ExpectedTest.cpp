/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#include <catch2/catch.hpp>
#include <SkyboltCommon/Expected.h>

using namespace skybolt;

TEST_CASE("Get value from Expected with value")
{
	Expected<int> e = 1;
	CHECK(valueOrLogWarning(e) == 1);
	CHECK(valueOrLogError(e) == 1);
	CHECK(valueOrThrowException(e) == 1);
}

TEST_CASE("Get value from Expected with error throws exception")
{
	Expected<int> e = UnexpectedMessage{"error message"};
	
	bool elseBranchCalled = false;
	auto result = valueOrElse(e, [&](const auto&) {
		elseBranchCalled = true;
		});
	CHECK(result == std::nullopt);
	CHECK(elseBranchCalled);

	CHECK_THROWS(valueOrThrowException(e));
}
