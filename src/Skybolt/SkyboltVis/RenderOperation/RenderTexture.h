/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#pragma once

#include "RenderTarget.h"
#include <osg/Texture2D>
#include <boost/signals2.hpp>
#include <functional>
#include <optional>

namespace skybolt {
namespace vis {

using TextureFactory = std::function<osg::ref_ptr<osg::Texture>(const osg::Vec2i&)>;

//! Creates a factory for creating textures to be displayed as a screen quad.
//! These textures do not use mipmaps and do not repeat.
TextureFactory createScreenTextureFactory(GLint internalFormat);

struct RenderTextureConfig
{
	std::vector<TextureFactory> colorTextureFactories;
	std::optional<TextureFactory> depthTextureFactory;
	int multisampleSampleCount = 0;
	bool clear = true;
};

class RenderTexture : public RenderTarget
{
public:
	RenderTexture(const RenderTextureConfig& config);
	~RenderTexture();

public: // RenderTarget interface
	void updatePreRender(const RenderContext& renderContext) override;

	std::vector<osg::ref_ptr<osg::Texture>> getOutputTextures() const override
	{
		return mColorTextures;
	}

	const osg::ref_ptr<osg::Texture>& getDepthTexture() const
	{
		return mDepthTexture;
	}

private:
	std::vector<osg::ref_ptr<osg::Texture>> mColorTextures;
	osg::ref_ptr<osg::Texture> mDepthTexture;
	std::vector<TextureFactory> mColorTextureFactories;
	std::optional<TextureFactory> mDepthTextureFactory;
	int mMultisampleSampleCount;
};

} // namespace vis
} // namespace skybolt
