/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltVis/SkyboltVisFwd.h"
#include "SkyboltVis/Renderable/ScreenQuad.h"
#include "SkyboltVis/RenderOperation/RenderOperation.h"

#include <osg/Texture2D>

namespace skybolt {
namespace vis {

class TextureGeneratorCullCallback;

class GpuTextureGenerator : public RenderOperation
{
public:
	GpuTextureGenerator(const osg::ref_ptr<osg::Texture2D>& texture, const osg::ref_ptr<osg::StateSet>& stateSet, bool generateMipMaps);
	~GpuTextureGenerator();

	void requestRegenerate();

	std::vector<osg::ref_ptr<osg::Texture>> getOutputTextures() const override
	{
		return { mTexture };
	}

private:
	osg::ref_ptr<TextureGeneratorCullCallback> mTextureGeneratorCullCallback;
	osg::ref_ptr<osg::Texture2D> mTexture;
	osg::ref_ptr<osg::Camera> mCamera;
	std::unique_ptr<ScreenQuad> mQuad;
};

} // namespace vis
} // namespace skybolt
