/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "ViewportHelpers.h"
#include "Viewport.h"
#include "RenderTargetSceneAdapter.h"
#include "RenderTexture.h"
#include "SkyboltVis/Rect.h"
#include "SkyboltVis/RenderContext.h"
#include "SkyboltVis/Renderable/ScreenQuad.h"
#include "SkyboltVis/Window/Window.h"

#include <osg/Camera>
#include <osg/Texture2DMultisample>
#include <osgViewer/Viewer>

namespace skybolt {
namespace vis {

static std::shared_ptr<ScreenQuad> createFullscreenQuad(const osg::ref_ptr<osg::Program>& program)
{
	osg::ref_ptr<osg::StateSet> stateSet = new osg::StateSet();
	stateSet->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
	stateSet->setMode(GL_CULL_FACE, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
	stateSet->setAttribute(program);
	return std::make_shared<ScreenQuad>(stateSet);
}

struct ScreenQuadRenderTarget : RenderTexture::Scene
{
	ScreenQuadRenderTarget(const std::shared_ptr<ScreenQuad>& quad) :
		mQuad(quad)
	{
	}

	osg::ref_ptr<osg::Node> getNode() override
	{
		return mQuad->_getNode();
	}

private:
	std::shared_ptr<ScreenQuad> mQuad;
};

osg::ref_ptr<RenderTarget> createAndAddViewportToWindow(Window& window, const osg::ref_ptr<osg::Program>& compositorProgram)
{
	osg::ref_ptr<Viewport> viewport = new Viewport();
	viewport->setScene(std::make_shared<ScreenQuadRenderTarget>(createFullscreenQuad(compositorProgram)));

	auto colorTextureFactory = createTextureFactory(GL_RGBA16F_ARB);

	RenderTextureConfig config;
	config.colorTextureFactory = [viewport, colorTextureFactory](const osg::Vec2i& size) {
		osg::ref_ptr<osg::Texture> texture = colorTextureFactory(size);
		viewport->getOrCreateStateSet()->setTextureAttributeAndModes(0, texture);
		return texture;
	};
	config.depthTextureFactory = createTextureFactory(GL_DEPTH_COMPONENT);
	config.multisampleSampleCount = osg::DisplaySettings::instance()->getNumMultiSamples();

	osg::ref_ptr<RenderTexture> texture = new RenderTexture(config);
	window.addRenderTarget(texture, RectF(0, 0, 1, 1));
	window.addRenderTarget(viewport, RectF(0, 0, 1, 1));
	return texture;
}

} // namespace vis
} // namespace skybolt
