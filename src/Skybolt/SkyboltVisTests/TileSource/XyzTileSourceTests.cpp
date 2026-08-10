/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include <catch2/catch.hpp>
#include <SkyboltVis/Elevation/ElevationBounds.h>
#include <SkyboltVis/Elevation/ElevationImageMetadata.h>
#include <SkyboltVis/Elevation/ElevationRerange.h>
#include <SkyboltVis/Image/SimpleImageFactory.h>
#include <SkyboltVis/Renderable/Planet/Tile/TileSource/XyzTileSource.h>

#include <filesystem>

using namespace skybolt;
using namespace skybolt::vis;

namespace fs = std::filesystem;

static fs::path getTemporaryDirectory()
{
	return fs::temp_directory_path() / "SkyboltTests";
}

static XyzTileSource createXyzTileSource(std::optional<ElevationRerange> elevationRerange = {})
{
	fs::create_directories(getTemporaryDirectory());

	XyzTileSourceConfig config;
	config.urlTemplate = (getTemporaryDirectory() / "{key}_{z}_{x}_{y}.png").string();
	config.apiKey = "testKey";
	config.levelRange = IntRangeInclusive(0, 2);
	config.elevationRerange = std::move(elevationRerange);
	config.imageFactory = std::make_shared<SimpleImageFactory>();

	return XyzTileSource(config);
}

static Expected<std::monostate> writeTestColorImage(const fs::path& filename)
{
	SimpleImageFactory factory;
	ImagePtr image = valueOrThrowException(factory.createImage(1, 1, Image::Format::RGBA8, Image::ColorSpace::Linear));
	return factory.writeImage(*image, filename.string());
}

static Expected<std::monostate> writeTestElevationImage(const fs::path& filename, std::uint16_t elevationValue)
{
	SimpleImageFactory factory;
	ImagePtr image = valueOrThrowException(factory.createImage(1, 1, Image::Format::R16, Image::ColorSpace::Linear));
	image->getRawData()[0] = elevationValue;
	return factory.writeImage(*image, filename.string());
}

TEST_CASE("Test tile loaded from XYZ tile source")
{
	XyzTileSource source = createXyzTileSource();

	// Load existing image succeeds
	valueOrThrowException(writeTestColorImage(getTemporaryDirectory() / "testKey_2_1_3.png"));
	CHECK(source.createImage(QuadTreeTileKey(2,1,3), [] { return false;}));

	// Load non-existing image fails
	CHECK(!source.createImage(QuadTreeTileKey(0,1,2), [] { return false;}));
}

TEST_CASE("Test height map tile loaded from XYZ elevation tile source")
{
	XyzTileSource source = createXyzTileSource(getDefaultEarthRerange());

	// Load existing image succeeds
	int elevationValue = 23;
	valueOrThrowException(writeTestElevationImage(getTemporaryDirectory() / "testKey_2_1_3.png", elevationValue));

	ImagePtr image = source.createImage(QuadTreeTileKey(2,1,3), [] { return false;});
	REQUIRE(image);

	auto metadata = getElevationImageMetadata(*image);
	REQUIRE(metadata);
	CHECK(metadata->rerange == getDefaultEarthRerange());
	CHECK(getColorValueForElevation(metadata->rerange, metadata->elevationBounds.x) == elevationValue);
	CHECK(getColorValueForElevation(metadata->rerange, metadata->elevationBounds.y) == elevationValue);

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