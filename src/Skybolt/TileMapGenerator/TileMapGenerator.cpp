/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "TileMapGenerator.h"
#include <SkyboltVis/OsgMathHelpers.h>
#include <SkyboltCommon/Exception.h>
#include <SkyboltCommon/Math/MathUtility.h>
#include <SkyboltCommon/Math/QuadTree.h>
#include <osgDB/WriteFile>
#include <boost/filesystem.hpp>

using namespace skybolt;

osg::Vec4f getColorBilinear(const osg::Image& image, const osg::Vec2f& uv)
{
	// Calculate coordinates
	osg::Vec2f coord = uv;
	coord.x() *= image.s();
	coord.y() *= image.t();

	int sMax = image.s() - 1;
	int tMax = image.t() - 1;

	coord.x() = skybolt::math::clamp(coord.x(), 0.0f, float(sMax));
	coord.y() = skybolt::math::clamp(coord.y(), 0.0f, float(tMax));

	int u0 = (int)coord.x();
	int u1 = std::min(u0 + 1, sMax);
	int v0 = (int)coord.y();
	int v1 = std::min(v0 + 1, tMax);

	// Calculate weights
	float fracU = coord.x() - u0;
	float fracV = coord.y() - v0;

	// Interpolate
	osg::Vec4f d00 = image.getColor(u0, v0);
	osg::Vec4f d10 = image.getColor(u1, v0);
	osg::Vec4f d01 = image.getColor(u0, v1);
	osg::Vec4f d11 = image.getColor(u1, v1);

	osg::Vec4f fracUVec(fracU, fracU, fracU, fracU);
	osg::Vec4f d0 = componentWiseLerp(d00, d10, fracUVec);
	osg::Vec4f d1 = componentWiseLerp(d01, d11, fracUVec);

	return componentWiseLerp(d0, d1, osg::Vec4f(fracV, fracV, fracV, fracV));
}

struct TileGenerator
{
	TileGenerator(const std::string& outputDirectory, const osg::Vec2i& tileDimensions, const std::vector<TileMapGeneratorLayer>& layers, Filtering filtering = Filtering::Bilinear) :
		outputDirectory(outputDirectory), tileDimensions(tileDimensions), layers(layers), filtering(filtering)
	{
		for (const TileMapGeneratorLayer& layer : layers)
		{
			layerResolutions.push_back(std::max(layer.image->s() / layer.bounds.size().x(), layer.image->t() / layer.bounds.size().y()));
		}
	}

	bool operator()(const DefaultTile<osg::Vec2d>& tile) const
	{
		osg::Image* srcImage = layers.back().image;

		osg::ref_ptr<osg::Image> image = new osg::Image();
		image->allocateImage(tileDimensions.x(), tileDimensions.y(), 1, srcImage->getPixelFormat(), srcImage->getDataType());

		float maxSrcResolution = 0;
		for (int y = 0; y < tileDimensions.y(); ++y)
		{
			for (int x = 0; x < tileDimensions.x(); ++x)
			{
				Box2d pixelBounds(osg::Vec2d(double(x) / double(tileDimensions.x()), double(y) / double(tileDimensions.y())),
								  osg::Vec2d(double(x+1) / double(tileDimensions.x()), double(y+1) / double(tileDimensions.y())));

				osg::Vec2d size = tile.bounds.size();
				pixelBounds.minimum = tile.bounds.minimum + componentWiseMultiply(pixelBounds.minimum, size);
				pixelBounds.maximum = tile.bounds.minimum + componentWiseMultiply(pixelBounds.maximum, size);

				bool pixelWritten = false;

				for (int i = layers.size() - 1; i >= 0; --i)
				{
					const TileMapGeneratorLayer& layer = layers[i];
					if (layer.bounds.intersects(pixelBounds))
					{// TODO: downsample filter
						maxSrcResolution = std::max(maxSrcResolution, layerResolutions[i]);

						osg::Vec2d pSrc = componentWiseDivide(pixelBounds.center() - layer.bounds.minimum, layer.bounds.size());
						osg::Vec4f c;
						switch (filtering)
						{
						case Filtering::NearestNeighbor:
							c = layer.image->getColor(pSrc); break;
						case Filtering::Bilinear:
							c = getColorBilinear(*layer.image, pSrc); break;
						default:
							assert(!"Not implemented");
						}
						image->setColor(c, x, y);
						pixelWritten = true;
						break;
					}
				}

				if (!pixelWritten)
				{
					image->setColor(osg::Vec4(0,0,0,0), x, y);
				}
			}
		}

		std::string path = outputDirectory + "/" + std::to_string(tile.key.level);
		boost::filesystem::create_directory(path);
		path += +"/" + std::to_string(tile.key.x);
		boost::filesystem::create_directory(path);

		path += "/" + std::to_string(tile.key.y) + ".png";

		printf("Writing %s\n", path.c_str());
		osgDB::writeImageFile(*image, path);

		float outputResolution = std::max(tileDimensions.x() / tile.bounds.size().x(), tileDimensions.y() / tile.bounds.size().y());
		return (maxSrcResolution > outputResolution);
	}

	const std::string& outputDirectory;
	const osg::Vec2i& tileDimensions;
	const std::vector<TileMapGeneratorLayer>& layers;
	std::vector<float> layerResolutions;
	Filtering filtering;
};

void generateTileMap(const std::string& outputDirectory, const osg::Vec2i& tileDimensions, const std::vector<TileMapGeneratorLayer>& layers, Filtering filtering)
{
	if (layers.empty())
	{
		throw skybolt::Exception("No tile map generator input layers layers");
	}

	if (!boost::filesystem::is_directory(outputDirectory))
	{
		throw skybolt::Exception("Output directory '" + outputDirectory + "' does not exist");
	}

	Box2d bounds(osg::Vec2d(-math::piD(), -math::halfPiD()), osg::Vec2d(0, math::halfPiD()));
	QuadTree<DefaultTile<osg::Vec2d>> treeLeft(createDefaultTile<osg::Vec2d>, QuadTreeTileKey(0, 0, 0), bounds);
	
	bounds = Box2d(osg::Vec2d(0, -math::halfPiD()), osg::Vec2d(math::piD(), math::halfPiD()));
	QuadTree<DefaultTile<osg::Vec2d>> treeRight(createDefaultTile<osg::Vec2d>, QuadTreeTileKey(0, 1, 0), bounds);

	TileGenerator tileGenerator(outputDirectory, tileDimensions, layers, filtering);

	treeLeft.subdivideRecursively(treeLeft.getRoot(), tileGenerator);
	treeRight.subdivideRecursively(treeRight.getRoot(), tileGenerator);
}
