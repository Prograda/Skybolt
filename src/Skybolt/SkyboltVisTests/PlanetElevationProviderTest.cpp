/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include <catch2/catch.hpp>

#define PX_SCHED_IMPLEMENTATION 1
#include <px_sched/px_sched.h>

#include <SkyboltCommon/MapUtility.h>
#include <SkyboltVis/Renderable/Planet/Tile/TileSource/TileSource.h>
#include <SkyboltVis/ElevationProvider/HeightMapElevationProvider.h>
#include <SkyboltVis/ElevationProvider/TilePlanetAltitudeProvider.h>
#include <SkyboltCommon/Eventually.h>
#include <SkyboltCommon/NumericComparison.h>

#include <optional>

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

	bool hasAnyChildren(const skybolt::QuadTreeTileKey& key) const override { return true; }

	const std::string& getCacheSha() const override
	{
		static std::string r;
		return r;
	}

	//! @returns the highest key with source data in the given key's ancestral hierarchy
	std::optional<skybolt::QuadTreeTileKey> getHighestAvailableLevel(const skybolt::QuadTreeTileKey& key) const { return key; }

	std::map<skybolt::QuadTreeTileKey, osg::ref_ptr<osg::Image>> images;
	mutable std::vector<skybolt::QuadTreeTileKey> requests;
};

constexpr double altitude = 234;

static osg::ref_ptr<osg::Image> createDummyImage()
{
	HeightMapElevationRerange rerange = rerangeElevationFromUInt16WithElevationBounds(0, 65535);

	auto image = new osg::Image;
	image->allocateImage(1, 1, 1, GL_LUMINANCE, GL_UNSIGNED_SHORT);
	uint16_t* p = reinterpret_cast<uint16_t*>(image->data());
	*p = getColorValueForElevation(rerange, altitude);

	setHeightMapElevationRerange(*image, rerange);

	return image;
}

TEST_CASE("Test SynchronousPlanetAltitudeProvider returns zero elevation when no images found")
{
	auto source = std::make_shared<DummyTileSource>();
	TilePlanetAltitudeProvider provider(source, 5);
	CHECK(provider.getAltitude(sim::LatLon(1, 2)) == 0.0);
}

TEST_CASE("Test SynchronousPlanetAltitudeProvider returns best available lod image if highest lod is not available")
{
	auto source = std::make_shared<DummyTileSource>();
	source->images[QuadTreeTileKey(1, 0, 0)] = createDummyImage();

	TilePlanetAltitudeProvider provider(source, 3);
	CHECK(provider.getAltitude(sim::LatLon(1.4, -3.0)) == altitude);
	CHECK(source->requests.size() == 3);
	CHECK(source->requests[0].level == 3);
	CHECK(source->requests[1].level == 2);
	CHECK(source->requests[2].level == 1);
}

TEST_CASE("Test SynchronousPlanetAltitudeProvider uses cached images when highest lod is available")
{
	auto source = std::make_shared<DummyTileSource>();
	source->images[QuadTreeTileKey(1, 0, 0)] = createDummyImage();

	TilePlanetAltitudeProvider provider(source, 1);
	CHECK(provider.getAltitude(sim::LatLon(1.4, -3.0)) == altitude);
	CHECK(source->requests.size() == 1);
	CHECK(provider.getAltitude(sim::LatLon(1.4,- 3.0)) == altitude);
	CHECK(source->requests.size() == 1); // requests is still 1, indicating that a cached image was used instead of querying the source.
}

TEST_CASE("Test SynchronousPlanetAltitudeProvider uses cached images when highest lod is unavailable")
{
	auto source = std::make_shared<DummyTileSource>();
	source->images[QuadTreeTileKey(1, 0, 0)] = createDummyImage();

	TilePlanetAltitudeProvider provider(source, 4);
	CHECK(provider.getAltitude(sim::LatLon(1.4, -3.0)) == altitude);
	CHECK(source->requests.size() == 4);
	CHECK(provider.getAltitude(sim::LatLon(1.4, -3.0)) == altitude);
	CHECK(source->requests.size() == 4); // requests is still 1, indicating that a cached image was used instead of querying the source.
}

TEST_CASE("Test AsyncPlanetAltitudeProvider background loads tile")
{
	auto source = std::make_shared<DummyTileSource>();
	source->images[QuadTreeTileKey(1, 0, 0)] = createDummyImage();

	px_sched::Scheduler scheduler;
	scheduler.init();

	TileAsyncPlanetAltitudeProvider provider(&scheduler, source, 3);
	CHECK(provider.getAltitudeOrRequestLoad(sim::LatLon(1.4, -3.0)) == std::nullopt);

	CHECK(eventually([&]{
		return provider.getAltitudeOrRequestLoad(sim::LatLon(1.4, -3.0)) == altitude;
	}));
}