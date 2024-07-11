/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <osg/Camera>
#include <osg/Texture2D>

#include <mutex>

namespace skybolt {
namespace vis {

//! By default, OSG uploads textures to GPU just-in-time, i.e in the frame they are first used.
//! A big texture data upload can causes a stutter.
//! TextureCompiler allows textures to be compiled (uploaded to GPU) before they are first used,
//! which can be used to avoid stutters.
class TextureCompiler : public osg::Camera::DrawCallback
{
public:
	TextureCompiler(osg::State* state);

	void operator() (osg::RenderInfo& renderInfo) const override;

	void enqueueTexture(osg::ref_ptr<osg::Texture2D> texture);

private:
	mutable std::vector<osg::ref_ptr<osg::Texture2D>> mTextures;
	mutable std::mutex mTexturesMutex;
	osg::State* mState;
};

} // namespace vis
} // namespace skybolt
