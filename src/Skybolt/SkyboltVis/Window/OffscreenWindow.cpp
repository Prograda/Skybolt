/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#include "OffscreenWindow.h"
#include "OffscreenViewer.h"
#include <osgViewer/ViewerEventHandlers>

using namespace skybolt::vis;

OffscreenWindowConfig createDefaultConfigWithDimensions(int width, int height)
{
	OffscreenWindowConfig c;
	c.width = width;
	c.height = height;
	return c;
}

OffscreenWindow::OffscreenWindow(int width, int height) :
	OffscreenWindow(createDefaultConfigWithDimensions(width, height))
{
}

OffscreenWindow::OffscreenWindow(const OffscreenWindowConfig& config) :
	Window(std::unique_ptr<osgViewer::Viewer>(createOffscreenViewer(config.width, config.height).release()), config.displaySettings),
	mWidth(config.width),
	mHeight(config.height)
{
	mViewer->realize();
	configureGraphicsState();

	// TODO: The Viewer's default camera clears the frame before our Camera is rendered from, which clears the frame a second time.
	// The first frame clearing is redundant and should be avoided.
}

int OffscreenWindow::getWidth() const
{
	return mWidth;
}

int OffscreenWindow::getHeight() const
{
	return mHeight;
}