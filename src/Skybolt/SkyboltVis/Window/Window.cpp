/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "Window.h"
#include "DisplaySettings.h"
#include "SkyboltVis/Camera.h"
#include "SkyboltVis/OsgLogHandler.h"
#include "SkyboltVis/RenderContext.h"
#include "SkyboltVis/RenderOperation/RenderTarget.h"

#include <boost/foreach.hpp>

#ifdef OSG_LIBRARY_STATIC
#include <osgDB/Registry>
// include the plugins we need
USE_OSGPLUGIN(bmp)
USE_OSGPLUGIN(curl)
USE_OSGPLUGIN(dds)
USE_OSGPLUGIN(freetype)
USE_OSGPLUGIN(jpeg)
USE_OSGPLUGIN(png)
USE_OSGPLUGIN(tga)

USE_OSGPLUGIN(osg2)
USE_SERIALIZER_WRAPPER_LIBRARY(osg)

// include the platform specific GraphicsWindow implementation
USE_GRAPHICSWINDOW()
#endif

namespace skybolt {
namespace vis {

Window::Window(std::unique_ptr<osgViewer::Viewer> viewer, const DisplaySettings& settings) :
	mViewer(std::move(viewer)),
	mRootGroup(new osg::Group),
	mRenderOperationSequence(std::make_unique<RenderOperationSequence>())
{
	assert(mViewer);
	mRootGroup->addChild(mRenderOperationSequence->getRootNode());

	forwardOsgLogToBoost();

	osg::DisplaySettings::instance()->setNumMultiSamples(settings.multiSampleCount);

	osg::setNotifyLevel(osg::WARN);
	mViewer->setKeyEventSetsDone(0); // disable default 'escape' key binding to quit the application
	mViewer->setThreadingModel(osgViewer::ViewerBase::SingleThreaded); // TODO: Use multi-threaded?

	osg::StateSet* stateSet = mRootGroup->getOrCreateStateSet();
	// We will write to the frame buffer in linear light space, and it will automatically convert to SRGB
	stateSet->setMode(GL_FRAMEBUFFER_SRGB, osg::StateAttribute::ON);

	stateSet->setMode(GL_CULL_FACE, osg::StateAttribute::ON);

	mViewer->setSceneData(osg::ref_ptr<osg::Node>(mRootGroup));

	mScreenSizePixelsUniform = new osg::Uniform("screenSizePixels", osg::Vec2f(0, 0));
	stateSet->addUniform(mScreenSizePixelsUniform);
}

Window::~Window()
{
}

bool Window::render(LoadTimingPolicy loadTimingPolicy)
{
	mScreenSizePixelsUniform->set(osg::Vec2f(getWidth(), getHeight()));

	RenderContext context;
	context.targetDimensions = osg::Vec2i(getWidth(), getHeight());
	context.loadTimingPolicy = loadTimingPolicy;
	mRenderOperationSequence->updatePreRender(context);

	mViewer->frame();

	return !mViewer->done();
}

void Window::configureGraphicsState()
{
	// Set global graphics state
	osg::GraphicsContext* context = mViewer->getCamera()->getGraphicsContext();
	if (!context)
	{
		throw std::runtime_error("Could not initialize graphics context");
	}
	osg::State* state = context->getState();

	state->setUseModelViewAndProjectionUniforms(true);
	state->setUseVertexAttributeAliasing(true);

	// Disable GL error checking because it is expensive (measured about 13% performance hit compared with osg::State::ONCE_PER_FRAME).
	// Enable to debug OpenGL.
	state->setCheckForGLErrors(osg::State::NEVER_CHECK_GL_ERRORS);
}

} // namespace vis
} // namespace skybolt