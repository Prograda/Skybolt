/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include <catch/catch.hpp>
#include <SkyboltCommon/MapUtility.h>
#include <SkyboltVis/Renderable/Planet/Tile/TileSource/TileSource.h>
#include <SkyboltVis/ElevationProvider/HeightmapElevationProvider.h>
#include <SkyboltVis/ElevationProvider/PlanetAltitudeProvider.h>
#include <SkyboltCommon/NumericComparison.h>

using namespace skybolt;
using namespace skybolt::vis;

class DummyTileSource : public TileSource
{
public:
	osg::ref_ptr<osg::Image> createImage(const skybolt::QuadTreeTileKey& key, std::function<bool()> cancelSupplier) const override
	{
		requests.push_back(key);
		auto result = skybolt::findOptional(images, key);
		if (result)
		{
			return *result;
		}
		return nullptr;
	}

	std::map<skybolt::QuadTreeTileKey, osg::ref_ptr<osg::Image>> images;
	mutable std::vector<skybolt::QuadTreeTileKey> requests;
};

constexpr double altitude = 234;

static osg::ref_ptr<osg::Image> createDummyImage()
{
	auto image = new osg::Image;
	image->allocateImage(1, 1, 1, GL_LUMINANCE, GL_UNSIGNED_SHORT);
	uint16_t* p = reinterpret_cast<uint16_t*>(image->data());
	*p = floatToHeightmapValue(-altitude);
	return image;
}

TEST_CASE("Test PlanetAltitudeProvider returns zero elevation when no images found")
{
	auto source = std::make_shared<DummyTileSource>();
	PlanetAltitudeProvider provider(source, 5);
	CHECK(provider.getAltitude(sim::LatLon(1, 2)) == 0.0);
}

TEST_CASE("Test PlanetAltitudeProvider returns best available lod image if highest lod is not available")
{
	auto source = std::make_shared<DummyTileSource>();
	source->images[QuadTreeTileKey(1, 0, 0)] = createDummyImage();

	PlanetAltitudeProvider provider(source, 3);
	CHECK(provider.getAltitude(sim::LatLon(1.4, -3.0)) == altitude);
	CHECK(source->requests.size() == 3);
	CHECK(source->requests[0].level == 3);
	CHECK(source->requests[1].level == 2);
	CHECK(source->requests[2].level == 1);
}

TEST_CASE("Test PlanetAltitudeProvider uses cached images when highest lod is available")
{
	auto source = std::make_shared<DummyTileSource>();
	source->images[QuadTreeTileKey(1, 0, 0)] = createDummyImage();

	PlanetAltitudeProvider provider(source, 1);
	CHECK(provider.getAltitude(sim::LatLon(1.4, -3.0)) == altitude);
	CHECK(source->requests.size() == 1);
	CHECK(provider.getAltitude(sim::LatLon(1.4,- 3.0)) == altitude);
	CHECK(source->requests.size() == 1); // requests is still 1, indicating that a cached image was used instead of querying the source.
}

TEST_CASE("Test PlanetAltitudeProvider uses cached images when highest lod is unavailable")
{
	auto source = std::make_shared<DummyTileSource>();
	source->images[QuadTreeTileKey(1, 0, 0)] = createDummyImage();

	PlanetAltitudeProvider provider(source, 4);
	CHECK(provider.getAltitude(sim::LatLon(1.4, -3.0)) == altitude);
	CHECK(source->requests.size() == 4);
	CHECK(provider.getAltitude(sim::LatLon(1.4, -3.0)) == altitude);
	CHECK(source->requests.size() == 4); // requests is still 1, indicating that a cached image was used instead of querying the source.
}
