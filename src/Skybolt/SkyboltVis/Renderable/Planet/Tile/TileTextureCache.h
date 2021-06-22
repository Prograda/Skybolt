/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <SkyboltVis/SkyboltVisFwd.h>
#include <SkyboltCommon/LruCacheMap.h>
#include <osg/Image>
#include <osg/Texture2D>
#include <functional>

namespace std {
template <typename T>
struct hash<osg::ref_ptr<T>>
{
	size_t operator()(const osg::ref_ptr<T>& p) const
	{
		return std::hash<T*>()(p.get());
	}
};
}

namespace skybolt {
namespace vis {

class TileTextureCache
{
public:
	TileTextureCache();
	~TileTextureCache();

	enum class TextureType
	{
		Height,
		Normal,
		LandMask,
		Albedo,
		Attribute,
		TypeCount
	};

	using TextureFactory = std::function<osg::ref_ptr<osg::Texture2D>(osg::ref_ptr<osg::Image>)>;
	osg::ref_ptr<osg::Texture2D> getOrCreateTexture(TextureType type, osg::ref_ptr<osg::Image> image, const TextureFactory& factory);

private:
	using TextureCache = LruCacheMap<osg::ref_ptr<osg::Image>, osg::ref_ptr<osg::Texture2D>>;
	constexpr static size_t lruCacheSize = 256;
	std::vector<TextureCache> mCaches;
};

} // namespace vis
} // namespace skybolt
