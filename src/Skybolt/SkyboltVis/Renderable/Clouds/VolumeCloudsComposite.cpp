/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "VolumeCloudsComposite.h"
#include "SkyboltVis/OsgStateSetHelpers.h"
#include "SkyboltVis/OsgTextureHelpers.h"
#include "SkyboltVis/Renderable/ScreenQuad.h"

namespace skybolt {
namespace vis {

static osg::StateSet* createTexturedQuadStateSet(osg::ref_ptr<osg::Program> program)
{
	osg::StateSet* stateSet = new osg::StateSet();
	stateSet->setAttributeAndModes(program, osg::StateAttribute::ON);
	stateSet->addUniform(createUniformSampler2d("colorTexture", 0));
	stateSet->addUniform(createUniformSampler2d("depthTexture", 1));
	stateSet->setMode(GL_CULL_FACE, osg::StateAttribute::OFF);
	return stateSet;
}

class VolumeCloudsComposite : public ScreenQuad
{
public:
	VolumeCloudsComposite(const osg::ref_ptr<osg::StateSet>& texturedQuadStateSet, const TextureProvider& colorTextureProvider, const TextureProvider& depthTextureProvider) :
		ScreenQuad(texturedQuadStateSet),
		mTexturedQuadStateSet(texturedQuadStateSet),
		mColorTextureProvider(colorTextureProvider),
		mDepthTextureProvider(depthTextureProvider)
	{
	}

	void updatePreRender(const CameraRenderContext& context) override
	{
		mTexturedQuadStateSet->setTextureAttributeAndModes(0, mColorTextureProvider(), osg::StateAttribute::ON);
		mTexturedQuadStateSet->setTextureAttributeAndModes(1, mDepthTextureProvider(), osg::StateAttribute::ON);
	}

private:
	osg::ref_ptr<osg::StateSet> mTexturedQuadStateSet;
	TextureProvider mColorTextureProvider;
	TextureProvider mDepthTextureProvider;
};

VisObjectPtr createVolumeCloudsComposite(const VolumeCloudsCompositeConfig& config)
{
	osg::StateSet* texturedQuadStateSet = createTexturedQuadStateSet(config.compositorProgram);
	makeStateSetTransparent(*texturedQuadStateSet, vis::TransparencyMode::PremultipliedAlpha, RenderBinId::Clouds);

	return std::make_shared<VolumeCloudsComposite>(texturedQuadStateSet, config.colorTextureProvider, config.depthTextureProvider);
}

} // namespace vis
} // namespace skybolt
