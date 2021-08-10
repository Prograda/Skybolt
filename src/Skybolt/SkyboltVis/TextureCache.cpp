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
