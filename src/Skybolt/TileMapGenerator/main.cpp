/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "TileMapGenerator.h"
#include <SkyboltVis/OsgImageHelpers.h>
#include <SkyboltVis/OsgMathHelpers.h>
#include <SkyboltCommon/Math/MathUtility.h>
#include <osgDB/ReadFile>

using namespace skybolt::vis;
using namespace skybolt;

Box2d getTileBounds(int x, int y, int numTilesX, int numTilesY)
{
	static Box2d planetLonLatBounds(osg::Vec2d(-math::piD(), -math::halfPiD()), osg::Vec2d(math::piD(), math::halfPiD()));

	Box2d bounds(osg::Vec2d(double(x) / double(numTilesX), double(y) / double(numTilesY)),
		osg::Vec2d(double(x + 1) / double(numTilesX), double(y + 1) / double(numTilesY)));

	osg::Vec2d size = planetLonLatBounds.size();
	bounds.minimum = planetLonLatBounds.minimum + math::componentWiseMultiply(bounds.minimum, size);
	bounds.maximum = planetLonLatBounds.minimum + math::componentWiseMultiply(bounds.maximum, size);

	return bounds;
}

constexpr int defaultHeightmapSeaLevelValue = 32767;

static osg::ref_ptr<osg::Image> loadRawImage16bit(const std::string& filename, int width, int height)
{
	std::ifstream f(filename, std::ios::in | std::ios::binary);
	if (!f.is_open())
		throw skybolt::Exception("Unable to open file: " + filename);

	osg::Image* image = new osg::Image;
	image->allocateImage(width, height, 1, GL_LUMINANCE, GL_UNSIGNED_SHORT);
	image->setInternalTextureFormat(GL_R16);

	size_t elementCount = width * height;
	size_t sizeBytes = elementCount * 2;
	f.read((char*)image->data(), sizeBytes);

	// Post process
	char* p = (char*)image->data();
	for (int i = 0; i < elementCount; ++i)
	{
		// Convert from big to little endian
		//std::swap(*p, *(p + 1));

		// Offset sea level
		uint16_t& value = *(uint16_t*)p;
		value += defaultHeightmapSeaLevelValue;

		p += 2;
	}

	f.close();
	return image;
}

void postProcessStrm(osg::Image& image)
{
	size_t elementCount = image.s() * image.t();
	for (size_t i = 0; i < elementCount; ++i)
	{
		uint16_t& value = ((uint16_t*)image.data())[i];

		if (value == 32768) // ocean mask
		{
			value = -500; // set to match mask in GLOBE data
		}
		value += defaultHeightmapSeaLevelValue;
	}
}

int main_nlcd()
{
	std::string outputDirectory = "nlcd_2011_landcover_2011_edition_2014_10_10/TMS";
	osg::Vec2i tileDimensions(256, 256);

	std::vector<TileMapGeneratorLayer> layers;

	// Add NLCD Seattle tile
	TileMapGeneratorLayer layer;
	layer.image = osgDB::readImageFile("nlcd_2011_landcover_2011_edition_2014_10_10/nlcd_2011_landcover_seattle.tif");
	layer.bounds = Box2d(osg::Vec2d(osg::DegreesToRadians(-124.0), osg::DegreesToRadians(45.0)), osg::Vec2d(osg::DegreesToRadians(-120.0), osg::DegreesToRadians(50.0)));
	layers.push_back(layer);

	std::cout << "Inputs loaded" << std::endl;

	try
	{
		generateTileMap(outputDirectory, tileDimensions, layers, Filtering::NearestNeighbor);
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int main_dem()
{
	std::string outputDirectory = "DEM/CombinedElevation";
	osg::Vec2i tileDimensions(256, 256);
	
	std::vector<TileMapGeneratorLayer> layers;

	// Add GLOBE tiles
	{
		std::string dir = "DEM/GLOBE";

		double latitudes[5] = {
			-math::halfPiD(),
			-math::halfPiD() + math::piD() * double(4800) / double(21600),
			-math::halfPiD() + math::piD() * double(10800) / double(21600),
			-math::halfPiD() + math::piD() * double(16800) / double(21600),
			math::halfPiD()
		};

		int i = 0;
		for (int y = 0; y < 4; ++y)
		{
			for (int x = 0; x < 4; ++x)
			{
				std::string filename = dir + "/" + std::string(1, char(int('a') + i)) + "10g";

				int width = 10800;
				int height = (y == 1 || y == 2) ? 6000 : 4800;

				TileMapGeneratorLayer layer;
				layer.image = loadRawImage16bit(filename, width, height);
				layer.image->flipVertical();
				layer.bounds = getTileBounds(x, 3-y, 4, 4);
				layer.bounds.minimum.y() = latitudes[3-y];
				layer.bounds.maximum.y() = latitudes[(3-y)+1];
				layers.push_back(layer);
				++i;
			}
		}
	}

	// Add STRM tiles
	{
		TileMapGeneratorLayer layer;
		layer.image = osgDB::readImageFile("DEM/STRM_90m_DEM4/srtm_12_03.tif");
		postProcessStrm(*layer.image);
		layer.bounds = Box2d(osg::Vec2d(osg::DegreesToRadians(-125.0), osg::DegreesToRadians(45.0)), osg::Vec2d(osg::DegreesToRadians(-120.0), osg::DegreesToRadians(50.0)));
		layers.push_back(layer);
	}
	{
		TileMapGeneratorLayer layer;
		layer.image = osgDB::readImageFile("DEM/STRM_90m_DEM4/srtm_14_06.tif");
		postProcessStrm(*layer.image);
		layer.bounds = Box2d(osg::Vec2d(osg::DegreesToRadians(-115.0), osg::DegreesToRadians(30.0)), osg::Vec2d(osg::DegreesToRadians(-110.0), osg::DegreesToRadians(35.0)));
		layers.push_back(layer);
	}


	std::cout << "Inputs loaded" << std::endl;

	try
	{
		generateTileMap(outputDirectory, tileDimensions, layers, Filtering::Bilinear);
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int main()
{
	return main_dem();
}