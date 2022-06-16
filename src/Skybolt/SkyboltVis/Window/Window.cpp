/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "Window.h"
#include "DisplaySettings.h"
#include "SkyboltVis/Camera.h"
#include "SkyboltVis/OsgLogHandler.h"
#include "SkyboltVis/RenderTarget/RenderTarget.h"

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

// include the platform specific GraphicsWindow implementation
USE_GRAPHICSWINDOW()
#endif

namespace skybolt {
namespace vis {

Window::Window(const DisplaySettings& settings) :
	mViewer(new osgViewer::Viewer),
	mRootGroup(new osg::Group)
{
	forwardOsgLogToBoost();

	osg::DisplaySettings::instance()->setNumMultiSamples(settings.multiSampleCount);

	osg::setNotifyLevel(osg::WARN);
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

bool Window::render()
{
	mScreenSizePixelsUniform->set(osg::Vec2f(getWidth(), getHeight()));

	mRenderOperationPipeline->updatePreRender();

	mViewer->frame();

	return !mViewer->done();
}

osg::Group* Window::getSceneGraphRoot() const
{
	return mViewer->getSceneData()->asGroup();
}

void Window::setRenderOperationPipeline(const RenderOperationPipelinePtr& renderOperationPipeline)
{
	if (mRenderOperationPipeline)
	{
		mRootGroup->removeChild(mRenderOperationPipeline->getRootNode());
	}
	
	mRenderOperationPipeline = renderOperationPipeline;
	
	if (mRenderOperationPipeline)
	{
		mRootGroup->addChild(mRenderOperationPipeline->getRootNode());
	}
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

osg::ref_ptr<RenderTarget> getFinalRenderTarget(const Window& window)
{
	const auto& operations = window.getRenderOperationPipeline()->getOperations();

	for (auto i = operations.rbegin(); i != operations.rend(); --i)
	{
		if (RenderTarget* target = dynamic_cast<RenderTarget*>(i->second.get()); target)
		{
			return target;
		}
	}
	return nullptr;
}

} // namespace vis
} // namespace skybolt