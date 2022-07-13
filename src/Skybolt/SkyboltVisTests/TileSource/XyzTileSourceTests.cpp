/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include <catch2/catch.hpp>
#include <SkyboltVis/Renderable/Planet/Tile/TileSource/XyzTileSource.h>
#include <SkyboltVis/Renderable/Planet/Tile/HeightMapElevationBounds.h>
#include <SkyboltVis/Renderable/Planet/Tile/HeightMapElevationRerange.h>

#include <osg/Image>
#include <osgDB/WriteFile>
#include <filesystem>

using namespace skybolt;
using namespace skybolt::vis;

namespace fs = std::filesystem;

static fs::path getTemporaryDirectory()
{
	return fs::temp_directory_path() / "SkyboltTests";
}

static XyzTileSource createXyzTileSource(XyzTileSourceConfig::ImageType type = XyzTileSourceConfig::ImageType::Color)
{
	fs::create_directories(getTemporaryDirectory());

	XyzTileSourceConfig config;
	config.urlTemplate = (getTemporaryDirectory() / "{key}_{z}_{x}_{y}.png").string();
	config.apiKey = "testKey";
	config.levelRange = IntRangeInclusive(0, 2);
	config.imageType = type;

	return XyzTileSource(config);
}

static void writeTestColorImage(const fs::path& filename)
{
	osg::ref_ptr<osg::Image> image = new osg::Image();
	image->allocateImage(1, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE);
	osgDB::writeImageFile(*image, filename.string());
}

static void writeTestElevationImage(const fs::path& filename, std::uint16_t elevationValue)
{
	osg::ref_ptr<osg::Image> image = new osg::Image();
	image->allocateImage(1, 1, 1, GL_LUMINANCE, GL_UNSIGNED_SHORT);
	*reinterpret_cast<std::uint16_t*>(image->data()) = elevationValue;
	osgDB::writeImageFile(*image, filename.string());
}

TEST_CASE("Test tile loaded from XYZ tile source")
{
	XyzTileSource source = createXyzTileSource();

	// Load existing image succeeds
	writeTestColorImage(getTemporaryDirectory() / "testKey_2_1_3.png");
	CHECK(source.createImage(QuadTreeTileKey(2,1,3), [] { return false;}));

	// Load non-existing image fails
	CHECK(!source.createImage(QuadTreeTileKey(0,1,2), [] { return false;}));
}

TEST_CASE("Test height map tile loaded from XYZ elevation tile source")
{
	XyzTileSource source = createXyzTileSource(XyzTileSourceConfig::ImageType::Elevation);

	// Load existing image succeeds
	int elevationValue = 23;
	writeTestElevationImage(getTemporaryDirectory() / "testKey_2_1_3.png", elevationValue);

	osg::ref_ptr<osg::Image> image = source.createImage(QuadTreeTileKey(2,1,3), [] { return false;});
	CHECK(image);

	auto rerange = getHeightMapElevationRerange(*image);
	REQUIRE(rerange);

	auto bounds = getHeightMapElevationBounds(*image);
	REQUIRE(bounds);
	CHECK(getColorValueForElevation(*rerange, bounds->x()) == elevationValue);
	CHECK(getColorValueForElevation(*rerange, bounds->y()) == elevationValue);

}

TEST_CASE("Test tile source only reports having children at valid levels")
{
	XyzTileSource source = createXyzTileSource();

	CHECK(source.hasAnyChildren(QuadTreeTileKey(0,0,0)));
	CHECK(source.hasAnyChildren(QuadTreeTileKey(1,0,0)));
	CHECK(!source.hasAnyChildren(QuadTreeTileKey(2,0,0)));
}

TEST_CASE("Test tile source reports available levels")
{
	XyzTileSource source = createXyzTileSource();

	// Key available at given level
	CHECK(source.getHighestAvailableLevel(QuadTreeTileKey(1,2,3)) == QuadTreeTileKey(1,2,3));

	// Key available at lower level
	CHECK(source.getHighestAvailableLevel(QuadTreeTileKey(3,2,2)) == QuadTreeTileKey(2,1,1));
}