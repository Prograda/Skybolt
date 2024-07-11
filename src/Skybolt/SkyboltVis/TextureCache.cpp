/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "TextureCache.h"

namespace skybolt {
namespace vis {

osg::ref_ptr<osg::Texture2D> TextureCache::getOrCreateTexture(const std::string& filename, const TextureFactory& factory)
{
	auto i = mCache.find(filename);
	if (i != mCache.end())
	{
		return i->second;
	}
	auto texture = factory(filename);
	mCache[filename] = texture;
	return texture;
}

} // namespace vis
} // namespace skybolt
