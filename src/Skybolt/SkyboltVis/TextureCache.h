/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <osg/Texture2D>

#include <functional>
#include <map>
#include <string>

namespace skybolt {
namespace vis {

class TextureCache
{
public:

	using TextureFactory = std::function<osg::ref_ptr<osg::Texture2D>(const std::string& filename)>;

	//! If texture does not exist in cache, it is created with factory and added to cache
	osg::ref_ptr<osg::Texture2D> getOrCreateTexture(const std::string& filename, const TextureFactory& factory);

private:
	std::map<std::string, osg::ref_ptr<osg::Texture2D>> mCache;
};

} // namespace vis
} // namespace skybolt
