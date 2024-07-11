/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "TileMapGenerator.h"
#include <SkyboltVis/OsgImageHelpers.h>
#include <SkyboltVis/OsgMathHelpers.h>
#include <SkyboltCommon/Exception.h>
#include <SkyboltCommon/Math/MathUtility.h>
#include <SkyboltCommon/Math/QuadTree.h>

#include <osgDB/WriteFile>
#include <filesystem>
#include <boost/noncopyable.hpp>

#define PX_SCHED_IMPLEMENTATION 1
#include <px_sched/px_sched.h>

using namespace skybolt;
using namespace vis;

static osg::Vec4f sampleImage(const osg::Image& image, const osg::Vec2d& point, Filtering filtering)
{
	switch (filtering)
	{
	case Filtering::NearestNeighbor:
		return image.getColor(point);
		break;
	case Filtering::Bilinear:
		return vis::getColorBilinear(image, osg::Vec2f(point.x() * image.s(), point.y() * image.t()));
		break;
	default:
		assert(!"Not implemented");
	}
	return osg::Vec4f();
}

struct TileGeneratorConfig
{
	std::unique_ptr<px_sched::Scheduler> scheduler;
	std::string outputDirectory;
	osg::Vec2i tileDimensions;
	std::vector<TileMapGeneratorLayer> layers;
	Filtering filtering;
	std::string extension;
	double mipmapBias = 0; // Higher numbers blur the image, lower numbers sharpen, 0 is neutral.
};

struct TileGenerator : public boost::noncopyable
{
	TileGenerator(TileGeneratorConfig config) :
		mScheduler(std::move(config.scheduler)),
		mOutputDirectory(std::move(config.outputDirectory)),
		mTileDimensions(std::move(config.tileDimensions)),
		mLayers(std::move(config.layers)),
		mFiltering(std::move(config.filtering)),
		mExtension(std::move(config.extension)),
		mMipmapBias(config.mipmapBias)
	{
		printf("Generating mipmaps...\n");
		for (const TileMapGeneratorLayer& layer : mLayers)
		{
			mLayerResolutions.push_back(std::max(layer.image->s() / layer.bounds.size().x(), layer.image->t() / layer.bounds.size().y()));
			mLayerImageMipmaps.push_back(generateMipmaps(layer.image));
		}
	}

	~TileGenerator()
	{
		mScheduler->waitFor(mLoadingTaskSync);
	}

	bool operator()(const DefaultTile<osg::Vec2d>& tile) const
	{
		if (mScheduler->num_tasks() > mScheduler->params().num_threads)
		{
			mScheduler->run([this, bounds = tile.bounds, key = tile.key]() {
				generateImage(bounds, key);
			}, &mLoadingTaskSync);
		}
		else // else all background threads are in use. Perform task on current thread.
		{
			generateImage(tile.bounds, tile.key);
		}

		// Find find the highest resolution of the layers that interesect the tile
		float maxSrcResolution = 0;
		for (int i = mLayers.size() - 1; i >= 0; --i)
		{
			const TileMapGeneratorLayer& layer = mLayers[i];
			if (layer.bounds.intersects(tile.bounds))
			{
				maxSrcResolution = std::max(maxSrcResolution, mLayerResolutions[i]);
			}
		}

		// If resolution is higher than current tile resolution, return true to indicate that further subdivision is required
		float outputResolution = std::max(mTileDimensions.x() / tile.bounds.size().x(), mTileDimensions.y() / tile.bounds.size().y());
		return (maxSrcResolution > outputResolution);
	}

	// @ThreadSafe
	void generateImage(const vis::Box2d& tileBounds, const QuadTreeTileKey& tileKey) const
	{
		osg::Image* srcImage = mLayers.back().image;

		osg::ref_ptr<osg::Image> image = new osg::Image();
		image->allocateImage(mTileDimensions.x(), mTileDimensions.y(), 1, srcImage->getPixelFormat(), srcImage->getDataType());

		for (int y = 0; y < mTileDimensions.y(); ++y)
		{
			for (int x = 0; x < mTileDimensions.x(); ++x)
			{
				Box2d destPixelBounds(osg::Vec2d(double(x) / double(mTileDimensions.x()), double(y) / double(mTileDimensions.y())),
								  osg::Vec2d(double(x+1) / double(mTileDimensions.x()), double(y+1) / double(mTileDimensions.y())));

				osg::Vec2d size = tileBounds.size();
				destPixelBounds.minimum = tileBounds.minimum + math::componentWiseMultiply(destPixelBounds.minimum, size);
				destPixelBounds.maximum = tileBounds.minimum + math::componentWiseMultiply(destPixelBounds.maximum, size);

				bool pixelWritten = false;

				for (int i = mLayers.size() - 1; i >= 0; --i)
				{
					const TileMapGeneratorLayer& layer = mLayers[i];
					if (layer.bounds.intersects(destPixelBounds))
					{
						vis::Box2d layerPixelBounds(
							math::componentWiseDivide(destPixelBounds.minimum - layer.bounds.minimum, layer.bounds.size()),
							math::componentWiseDivide(destPixelBounds.maximum - layer.bounds.minimum, layer.bounds.size())
						);

						osg::ref_ptr<osg::Image> mipmap = getBestMipmap(mLayerImageMipmaps[i], layerPixelBounds.size());
						osg::Vec4f c = sampleImage(*mipmap, layerPixelBounds.center(), mFiltering);
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

		std::string path = mOutputDirectory + "/" + std::to_string(tileKey.level);
		std::filesystem::create_directory(path);
		path += +"/" + std::to_string(tileKey.x);
		std::filesystem::create_directory(path);

		path += "/" + std::to_string(tileKey.y) + "." + mExtension;

		bool verboseOutput = false;
		if (verboseOutput)
		{
			printf("Writing %s\n", path.c_str());
		}

		osgDB::writeImageFile(*image, path);

		{
			int filesWrittenCount = mFilesWrittenCount++;
			if ((filesWrittenCount % 1000) == 0)
			{
				printf("%i files written so far. Most recent file written: '%s'\n", filesWrittenCount, path.c_str());
			}
		}
	}

	using ImageMipmaps = std::vector<osg::ref_ptr<osg::Image>>; // Base image, followed by mipmaps in decreasing resolution
	osg::ref_ptr<osg::Image> getBestMipmap(const ImageMipmaps& mipmaps, const osg::Vec2d& texelSizeNdc) const
	{
		osg::Vec2d texelSizePixels = math::componentWiseMultiply(texelSizeNdc, osg::Vec2d(mipmaps.front()->s(), mipmaps.front()->t()));
		double dotProduct = texelSizePixels * texelSizePixels;
		int level = std::clamp(int(0.5 * std::log2(dotProduct) + mMipmapBias), 0, int(mipmaps.size()) - 1);
		return mipmaps[level];
	}

private:
	mutable std::unique_ptr<px_sched::Scheduler> mScheduler;
	const std::string mOutputDirectory;
	const osg::Vec2i mTileDimensions;
	const std::vector<TileMapGeneratorLayer> mLayers;
	const Filtering mFiltering;
	const std::string mExtension;
	const double mMipmapBias;

	std::vector<float> mLayerResolutions;
	
	std::vector<ImageMipmaps> mLayerImageMipmaps; //!< Image mipmaps for each item in mLayers
	
	mutable px_sched::Sync mLoadingTaskSync;
	mutable std::atomic_int mFilesWrittenCount = 0;
};

void generateTileMap(const std::string& outputDirectory, const osg::Vec2i& tileDimensions, const std::vector<TileMapGeneratorLayer>& layers, Filtering filtering, const std::string& extension)
{
	if (layers.empty())
	{
		throw skybolt::Exception("No tile map generator input layers layers");
	}

	if (!std::filesystem::exists(outputDirectory))
	{
		if (!std::filesystem::create_directories(outputDirectory))
		{
			throw skybolt::Exception("Could not create output directory '" + outputDirectory + "'");
		}
	}

	Box2d bounds(osg::Vec2d(-math::piD(), -math::halfPiD()), osg::Vec2d(0, math::halfPiD()));
	QuadTree<DefaultTile<osg::Vec2d>> treeLeft(createDefaultTile<osg::Vec2d>, QuadTreeTileKey(0, 0, 0), bounds);
	
	bounds = Box2d(osg::Vec2d(0, -math::halfPiD()), osg::Vec2d(math::piD(), math::halfPiD()));
	QuadTree<DefaultTile<osg::Vec2d>> treeRight(createDefaultTile<osg::Vec2d>, QuadTreeTileKey(0, 1, 0), bounds);

	auto scheduler = std::make_unique<px_sched::Scheduler>();
	int coreCount = std::thread::hardware_concurrency() - 1;
	px_sched::SchedulerParams schedulerParams;
	schedulerParams.max_running_threads = coreCount;
	schedulerParams.num_threads = coreCount;
	scheduler->init(schedulerParams);

	TileGenerator tileGenerator([&] {
		TileGeneratorConfig c;
		c.scheduler = std::move(scheduler);
		c.outputDirectory = outputDirectory;
		c.tileDimensions = tileDimensions;
		c.layers = layers;
		c.filtering = filtering;
		c.extension = extension;

		// Slight mipmap bias to sharpen images slightly. Prevents over blurring.
		// FIXME: this should be set to 0, but it's currently -1 as a workaround for mipmaps
		// being slightly shifted and overly blurred due to non-power of two source textures.
		// With a bias of -1 currently the mipmaps are slightly overly sharpened.
		//c.mipmapBias = -1;
		return c;
		}());

	QuadTree<DefaultTile<osg::Vec2d>>::SubdivisionPredicate predicate = [&] (const DefaultTile<osg::Vec2d>& tile) {
		return tileGenerator(tile);
	};

	treeLeft.subdivideRecursively(treeLeft.getRoot(), predicate);
	//treeRight.subdivideRecursively(treeRight.getRoot(), predicate);
}

static int nextPowerOfTwo(int v)
{
	// From https://stackoverflow.com/questions/4398711/round-to-the-nearest-power-of-two
	v--;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	v++;
	return v;
}

//! Creates an copy of the image with new dimensions half the original, scaled up to next power of two.
//! It's important the the resulting image is power of two, as non-power of two images in a mipmap chain may result
//! in repeated resizing which introduces blurring and shifting.
//! FIXME: scaling up to next power of two can make image appear sharper than expected for the given mipmap level.
osg::ref_ptr<osg::Image> createNearestHalfSizePowerOfTwoCopy(const osg::Image& image)
{
	osg::ref_ptr<osg::Image> result(new osg::Image);
	int outputWidth = std::max(1, nextPowerOfTwo(image.s() / 2));
	int outputHeight = std::max(1, nextPowerOfTwo(image.t() / 2));
	result->allocateImage(outputWidth, outputHeight, 1, image.getPixelFormat(), image.getDataType());

	float scaleX = float(image.s()) / outputWidth;
	float scaleY = float(image.t()) / outputHeight;

	for (int y = 0; y < outputHeight; ++y)
	{
		for (int x = 0; x < outputWidth; ++x)
		{
			// Find the pixel coordinate in the source image.
			// Note that scaleX and scaleY may be slightly larger than 2 if the source image has an odd number of pixels,
			// in which case std::round is important to ensure the nearest coordinate is used to minimize shifting in the image.
			int sx0 = std::min(image.s()-1, int(std::round(float(x)*scaleX)));
			int sy0 = std::min(image.t()-1, int(std::round(float(y)*scaleY)));
			int sx1 = std::min(image.s()-1, sx0+1);
			int sy1 = std::min(image.t()-1, sy0+1);

			osg::Vec4f c00 = image.getColor(sx0, sy0);
			osg::Vec4f c10 = image.getColor(sx1, sy0);
			osg::Vec4f c01 = image.getColor(sx0, sy1);
			osg::Vec4f c11 = image.getColor(sx1, sy1);
			osg::Vec4f color = (c00 + c10 + c01 + c11) * 0.25;
			setPixelColor(*result, x, y, color);
		}
	}
	return result;
}

std::vector<osg::ref_ptr<osg::Image>> generateMipmaps(const osg::ref_ptr<osg::Image>& base)
{
	assert(base);
	std::vector<osg::ref_ptr<osg::Image>> result = { base };

	while (true)
	{
		const osg::Image& prevImage = *result.back();
		int maxDim = std::max(prevImage.s(), prevImage.t());
		if (maxDim <= 1)
		{
			break;
		}
		result.push_back(createNearestHalfSizePowerOfTwoCopy(prevImage));
	}
	return result;
}