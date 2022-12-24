/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "Window.h"
#include "SkyboltVis/Camera.h"
#include "SkyboltVis/OsgLogHandler.h"
#include "SkyboltVis/RenderContext.h"
#include "SkyboltVis/RenderOperation/RenderTarget.h"

#include <assert.h>
#include <boost/foreach.hpp>

namespace skybolt {
namespace vis {

class NodeCallbackFunction : public osg::NodeCallback
{
public:
	NodeCallbackFunction(std::function<void()> fn) :
		mFn(std::move(fn))
	{
	}

	void operator()(osg::Node* node, osg::NodeVisitor* nv)
	{
		mFn();
		traverse(node, nv);
	}

private:
	std::function<void()> mFn;
};

Window::Window(const osg::ref_ptr<osgViewer::View>& view) :
	mView(view),
	mRenderOperationSequence(std::make_unique<RenderOperationSequence>())
{
	assert(view);

	mView->getCamera()->setClearMask(0); // No need to clear this camera, since the RenderOperationSequence will clear the view as needed
	mView->setSceneData(mRenderOperationSequence->getRootNode());

	osg::StateSet* stateSet = mView->getCamera()->getOrCreateStateSet();
	// We will write to the frame buffer in linear light space, and it will automatically convert to SRGB
	stateSet->setMode(GL_FRAMEBUFFER_SRGB, osg::StateAttribute::ON);

	stateSet->setMode(GL_CULL_FACE, osg::StateAttribute::ON);

	mScreenSizePixelsUniform = new osg::Uniform("screenSizePixels", osg::Vec2f(0, 0));
	stateSet->addUniform(mScreenSizePixelsUniform);

	// Call preRender callback during the cull phase, which happens just before render phase per camera.
	mView->getCamera()->addCullCallback(new NodeCallbackFunction([this] {
		preRender(mLoadTimingPolicy);
	}));
}

Window::~Window() = default;

void Window::preRender(LoadTimingPolicy policy)
{
	mScreenSizePixelsUniform->set(osg::Vec2f(getWidth(), getHeight()));

	RenderContext context;
	context.targetDimensions = osg::Vec2i(getWidth(), getHeight());
	context.frameNumber = mFrameNumber++;
	context.loadTimingPolicy = policy;
	mRenderOperationSequence->updatePreRender(context);
}

void configureGraphicsState(osg::GraphicsContext& context)
{
	// Set global graphics state
	osg::State* state = context.getState();

	state->setUseModelViewAndProjectionUniforms(true);
	state->setUseVertexAttributeAliasing(true);

	// Disable GL error checking because it is expensive (measured about 13% performance hit compared with osg::State::ONCE_PER_FRAME).
	// Enable to debug OpenGL.
	state->setCheckForGLErrors(osg::State::NEVER_CHECK_GL_ERRORS);
}

} // namespace vis
} // namespace skybolt