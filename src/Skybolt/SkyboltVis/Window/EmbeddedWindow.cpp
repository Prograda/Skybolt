/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#include "EmbeddedWindow.h"
#include <osgViewer/ViewerEventHandlers>

#include <assert.h>

using namespace skybolt::vis;

static osg::ref_ptr<osgViewer::View> createEmbeddedView(const RectI& rect)
{
	osg::ref_ptr<osgViewer::View> view = new osgViewer::View;

    osgViewer::GraphicsWindowEmbedded* context = new osgViewer::GraphicsWindowEmbedded(rect.x, rect.y, rect.width, rect.height);
    view->getCamera()->setViewport(new osg::Viewport(rect.x, rect.y, rect.width, rect.height));
    view->getCamera()->setGraphicsContext(context);

	configureGraphicsState(*context);
	return view;
}

EmbeddedWindow::EmbeddedWindow(const EmbeddedWindowConfig& config) :
	Window(createEmbeddedView(config.rect))
{
	const auto& rect = config.rect;

	mWindow = dynamic_cast<osgViewer::GraphicsWindowEmbedded*>(mView->getCamera()->getGraphicsContext());
	assert(mWindow);
}

void EmbeddedWindow::setWidth(int newWidth)
{
	int x, y, width, height;
	mWindow->getWindowRectangle(x, y, width, height);
	mWindow->setWindowRectangle(x, y, newWidth, height);

	mWindow->resized(x, y, newWidth, height);
}

void EmbeddedWindow::setHeight(int newHeight)
{
	int x, y, width, height;
	mWindow->getWindowRectangle(x, y, width, height);
	mWindow->setWindowRectangle(x, y, width, newHeight);

	mWindow->resized(x, y, width, newHeight);
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