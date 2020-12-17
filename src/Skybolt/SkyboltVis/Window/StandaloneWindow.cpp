/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "StandaloneWindow.h"
#include <osgViewer/ViewerEventHandlers>
#if defined(WIN32)
#include <osgViewer/api/Win32/GraphicsHandleWin32>
#else
#include <osgViewer/api/X11/GraphicsHandleX11>
#endif

using namespace skybolt::vis;

StandaloneWindow::StandaloneWindow(const RectI& rect)
{
	osg::DisplaySettings::instance()->setNumMultiSamples(4);

    mViewer->setUpViewInWindow(rect.x, rect.y, rect.width, rect.height);
	mViewer->realize();

	configureGraphicsState();

	// Hide cursor
	osgViewer::ViewerBase::Windows windows;
	mViewer->getWindows(windows);
	windows.front()->useCursor(false);

	// FIXME: The Viewer's default camera clears the frame before our Camera is rendered from, which clears the frame a second time.
	// The first frame clearing is unnecessary and should be eliminated.

}

int StandaloneWindow::getWidth() const
{
	osgViewer::ViewerBase::Windows windows;
	mViewer->getWindows(windows);
	if (windows.empty())
		return 0;

	int x, y, width, height;
	windows[0]->getWindowRectangle(x, y, width, height);
	return width;

}

int StandaloneWindow::getHeight() const
{
	osgViewer::ViewerBase::Windows windows;
	mViewer->getWindows(windows);
	if (windows.empty())
		return 0;

	int x, y, width, height;
	windows[0]->getWindowRectangle(x, y, width, height);
	return height;
}

std::string StandaloneWindow::getHandle() const
{
	osgViewer::ViewerBase::Windows windows;
	mViewer->getWindows(windows);
	if (windows.empty())
		return "";
#if defined(WIN32)
	size_t ptr = (size_t)dynamic_cast<osgViewer::GraphicsHandleWin32*>(windows[0])->getHWND();
#else
	size_t ptr = (size_t)dynamic_cast<osgViewer::GraphicsHandleX11*>(windows[0])->getDisplay();
#endif
	return std::to_string(ptr);
}
