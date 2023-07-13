/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include <catch2/catch.hpp>
#include <SkyboltVis/OsgImageHelpers.h>

#include <osg/Image>
#include <osg/ValueObject>
#include <sstream>

using namespace skybolt;
using namespace skybolt::vis;

TEST_CASE("Read and write image with user data")
{
	// Create image with user data
	osg::ref_ptr<osg::Image> image = new osg::Image();
	image->allocateImage(1, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE);
	image->setColor(osg::Vec4(0.1f, 0.2f, 0.3f, 0.4f), 0, 0);
	image->setUserValue("testData", 123);

	// Write image to stream
	std::stringstream ss;
	writeImageWithUserData(*image, ss, "png");

	// Read image from stream
	ss.seekg(0, std::ios::beg);
	osg::ref_ptr<osg::Image> image2 = readImageWithUserData(ss, "png");
	REQUIRE(image2);

	// Check that images match
	CHECK(image2->s() == 1);
	CHECK(image2->t() == 1);
	CHECK(image2->getPixelFormat() == GL_RGBA);
	int userValue;
	CHECK(image2->getUserValue("testData", userValue));
	CHECK(userValue == 123);
}