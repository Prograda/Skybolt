/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "Window.h"
#include "DisplaySettings.h"
#include "SkyboltVis/Camera.h"
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

using namespace skybolt::vis;

Window::Window(const DisplaySettings& settings) :
	mViewer(new osgViewer::Viewer)
{
	osg::DisplaySettings::instance()->setNumMultiSamples(settings.multiSampleCount);

	osg::setNotifyLevel(osg::WARN);
	mViewer->setThreadingModel(osgViewer::ViewerBase::SingleThreaded); // TODO: Use multi-threaded?

	osg::Group* rootGroup = new osg::Group;

	osg::StateSet* stateSet = rootGroup->getOrCreateStateSet();
	// We will write to the frame buffer in linear light space, and it will automatically convert to SRGB
	stateSet->setMode(GL_FRAMEBUFFER_SRGB, osg::StateAttribute::ON);

	stateSet->setMode(GL_CULL_FACE, osg::StateAttribute::ON);

	mViewer->setSceneData(osg::ref_ptr<osg::Node>(rootGroup));

	mScreenSizePixelsUniform = new osg::Uniform("screenSizePixels", osg::Vec2f(0, 0));
	stateSet->addUniform(mScreenSizePixelsUniform);
}

Window::~Window()
{
}

bool Window::render()
{
	for (const auto& item : mTargets)
	{
		const RectF& rectF = item.rect;
		RectI rect(getWidth() * rectF.x, getHeight() * rectF.y, getWidth() * rectF.width, getHeight() * rectF.height);
		item.target->setRect(rect);
	}

	mScreenSizePixelsUniform->set(osg::Vec2f(getWidth(), getHeight()));
	mViewer->frame();

	return !mViewer->done();
}

void Window::addRenderTarget(const osg::ref_ptr<RenderTarget>& target, const RectF& rect)
{
	mTargets.push_back(Target(target, rect));
	getSceneGraphRoot()->addChild(target);
}

void Window::removeRenderTarget(const osg::ref_ptr<RenderTarget>& target)
{
	getSceneGraphRoot()->removeChild(target);
	for (auto i = mTargets.begin(); i < mTargets.end(); ++i)
	{
		mTargets.erase(i);
		break;
	}
}

osg::Group* Window::getSceneGraphRoot() const
{
	return mViewer->getSceneData()->asGroup();
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