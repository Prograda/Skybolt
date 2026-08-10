/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "TileTextureCache.h"
#include "SkyboltVisOsg/OsgImage.h"

#include <SkyboltCommon/Exception.h>

using namespace skybolt;

namespace skybolt {
namespace vis {

TileTextureCache::TileTextureCache()
{
	for (size_t i = 0; i < size_t(TextureType::TypeCount); ++i)
	{
		mCaches.push_back(TextureCache(lruCacheSize));
	}
}

TileTextureCache::~TileTextureCache() = default;

osg::ref_ptr<osg::Texture2D> TileTextureCache::getOrCreateTexture(TextureType type, osg::ref_ptr<osg::Image> image, const TextureFactory& factory)
{
	auto& cache = mCaches[(size_t)type];

	osg::ref_ptr<osg::Texture2D> texture;

	if (!cache.get(image, texture))
	{
		texture = factory(image);
		cache.put(image, texture);
	};

	return texture;
}

osg::ref_ptr<osg::Texture2D> TileTextureCache::getOrCreateTexture(TextureType type, const ImagePtr& image, const TextureFactory& factory)
{
	const auto osgImage = std::dynamic_pointer_cast<OsgImage>(image);
	if (!osgImage)
	{
		if (!image)
		{
			throw skybolt::Exception("TileTextureCache requires valid, but null image was provided");
		}
		throw skybolt::Exception("TileTextureCache requires OsgImage, but image of class '" + std::string(typeid(*image).name()) + "' was provided");
	}
	return getOrCreateTexture(type, osgImage->getOsgImage(), factory);
}

} // namespace vis
} // namespace skybolt
