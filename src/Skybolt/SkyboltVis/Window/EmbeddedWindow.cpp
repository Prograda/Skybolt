/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#include "EmbeddedWindow.h"
#include <osgViewer/ViewerEventHandlers>

using namespace skybolt::vis;

EmbeddedWindow::EmbeddedWindow(const EmbeddedWindowConfig& config) :
	Window(std::make_unique<osgViewer::Viewer>(), config.displaySettings)
{
	const auto& rect = config.rect;
    mWindow = mViewer->setUpViewerAsEmbeddedInWindow(rect.x, rect.y, rect.width, rect.height);

	mViewer->realize();
	configureGraphicsState();

	// TODO: The Viewer's default camera clears the frame before our Camera is rendered from, which clears the frame a second time.
	// The first frame clearing is redundant and should be avoided.
}

void EmbeddedWindow::setWidth(int newWidth)
{
	int x, y, width, height;
	mWindow->getWindowRectangle(x, y, width, height);
	mWindow->setWindowRectangle(x, y, newWidth, height);

	mViewer->getCamera()->getGraphicsContext()->resized(x, y, newWidth, height);
}

void EmbeddedWindow::setHeight(int newHeight)
{
	int x, y, width, height;
	mWindow->getWindowRectangle(x, y, width, height);
	mWindow->setWindowRectangle(x, y, width, newHeight);

	mViewer->getCamera()->getGraphicsContext()->resized(x, y, width, newHeight);
}

int EmbeddedWindow::getWidth() const
{
	int x, y, width, height;
	mWindow->getWindowRectangle(x, y, width, height);
	return width;
}

int EmbeddedWindow::getHeight() const
{
	int x, y, width, height;
	mWindow->getWindowRectangle(x, y, width, height);
	return height;
}