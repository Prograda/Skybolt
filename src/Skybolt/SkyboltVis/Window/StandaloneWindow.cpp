/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "StandaloneWindow.h"
#include <osgViewer/View>

#if defined(WIN32)
#include <osgViewer/api/Win32/GraphicsHandleWin32>
#else
#include <osgViewer/api/X11/GraphicsWindowX11>
#endif

#include <assert.h>

using namespace skybolt::vis;

static StandaloneWindowConfig createDefaultConfigWithRect(const RectI& rect)
{
	StandaloneWindowConfig c;
	c.rect = rect;
	return c;
}

StandaloneWindow::StandaloneWindow(const RectI& rect) :
	StandaloneWindow(createDefaultConfigWithRect(rect))
{
}

static osg::ref_ptr<osgViewer::View> createStandaloneView(const RectI& rect)
{
	// TODO Use the vsync value in skybolt::DisplaySettings to enable/disable vsync by setting the value in graphics traits before creating the context.

	osg::ref_ptr<osgViewer::View> view = new osgViewer::View;
	view->setUpViewInWindow(rect.x, rect.y, rect.width, rect.height);

	osg::GraphicsContext* context = view->getCamera()->getGraphicsContext();
	if (!context)
	{
		throw std::runtime_error("Could not initialize graphics context");
	}

	configureGraphicsState(*context);
	return view;
}

StandaloneWindow::StandaloneWindow(const StandaloneWindowConfig& config) :
	Window(createStandaloneView(config.rect))
{
	const auto& rect = config.rect;

	getGraphicsWindow().useCursor(false);

	// FIXME: The Viewer's default camera clears the frame before our Camera is rendered from, which clears the frame a second time.
	// The first frame clearing is unnecessary and should be eliminated.
}

int StandaloneWindow::getWidth() const
{
	int x, y, width, height;
	getGraphicsWindow().getWindowRectangle(x, y, width, height);
	return width;
}

int StandaloneWindow::getHeight() const
{
	int x, y, width, height;
	getGraphicsWindow().getWindowRectangle(x, y, width, height);
	return height;
}

std::string StandaloneWindow::getHandle() const
{
	osg::GraphicsContext* context = mView->getCamera()->getGraphicsContext();
#if defined(WIN32)
	size_t ptr = (size_t)dynamic_cast<osgViewer::GraphicsHandleWin32*>(context)->getHWND();
#else
	size_t ptr = (size_t)dynamic_cast<osgViewer::GraphicsWindowX11*>(context)->getWindow();
#endif
	return std::to_string(ptr);
}

osgViewer::GraphicsWindow& StandaloneWindow::getGraphicsWindow() const
{
	osgViewer::GraphicsWindow* window = dynamic_cast<osgViewer::GraphicsWindow*>(mView->getCamera()->getGraphicsContext());
	assert(window);
	return *window;
}