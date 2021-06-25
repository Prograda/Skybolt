/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#include <catch2/catch.hpp>
#include <SkyboltCommon/ShaUtility.h>

using namespace skybolt;

TEST_CASE("Calc SHA1 of string")
{
	std::string sha = calcSha1("Hello world!");

	// Tested against http://www.sha1-online.com/
	CHECK(sha == "d3486ae9136e7856bc42212385ea797094475802");
}
