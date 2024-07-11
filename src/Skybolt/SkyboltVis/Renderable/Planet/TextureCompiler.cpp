/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "TextureCompiler.h"

namespace skybolt {
namespace vis {

TextureCompiler::TextureCompiler(osg::State* state) : mState(state) {}

void TextureCompiler::operator() (osg::RenderInfo& renderInfo) const
{
	std::vector<osg::ref_ptr<osg::Texture2D>> textures;
	{
		std::lock_guard<std::mutex> lock(mTexturesMutex);
		std::swap(textures, mTextures);
	}

	for (const osg::ref_ptr<osg::Texture2D> texture : textures)
	{
		texture->compileGLObjects(*mState);
	}
}

void TextureCompiler::enqueueTexture(osg::ref_ptr<osg::Texture2D> texture)
{
	std::lock_guard<std::mutex> lock(mTexturesMutex);
	mTextures.push_back(texture);
}

} // namespace vis
} // namespace skybolt
