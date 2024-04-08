/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "OsgWindow.h"
#include <SkyboltVis/VisRoot.h>
#include <SkyboltVis/Window/EmbeddedWindow.h>

#include <osgViewer/ViewerBase>

#ifdef WIN32
#include <osgViewer/api/win32/GraphicsWindowWin32>
#endif

#include <QKeyEvent>

using namespace skybolt;
using namespace skybolt::vis;

static osg::ref_ptr<osgViewer::View> createView(int width, int height, std::size_t hwnd, bool vsync)
{
	osg::ref_ptr<osg::GraphicsContext::Traits> traits(new osg::GraphicsContext::Traits);
	traits->x = 0;
	traits->y = 0;
	traits->width = width;
	traits->height = height;
	traits->red = 8;
	traits->green = 8;
	traits->blue = 8;
	traits->alpha = 8;
	traits->depth = 24;
	traits->windowDecoration = false;
	traits->doubleBuffer = true;
	traits->sharedContext = 0x0;
#ifdef WIN32
	traits->inheritedWindowData = new osgViewer::GraphicsWindowWin32::WindowData(HWND(hwnd));
#endif
	// FIXME: There's a bug in OSG where vsync is left at OS default when vsync=false, not actually set to false.
	// See https://github.com/openscenegraph/OpenSceneGraph/blob/master/src/osgViewer/GraphicsWindowWin32.cpp#L1978
	traits->vsync = vsync;

	osg::ref_ptr<osg::GraphicsContext> context = osg::GraphicsContext::createGraphicsContext(traits.get());
	configureGraphicsState(*context);

	osg::ref_ptr<osgViewer::View> view = new osgViewer::View;
	view->getCamera()->setViewport(new osg::Viewport(0, 0, width, height));
	view->getCamera()->setGraphicsContext(context);
	return view;
}

class OsgViewWindow : public Window
{
public:
	OsgViewWindow(const osg::ref_ptr<osgViewer::View>& view) :
		Window(view)
	{
	}

	int getWidth() const override
	{
		int x, y, width, height;
		getGraphicsWindow().getWindowRectangle(x, y, width, height);
		return width;
	}

	int getHeight() const override
	{
		int x, y, width, height;
		getGraphicsWindow().getWindowRectangle(x, y, width, height);
		return height;
	}

	osgViewer::GraphicsWindow& getGraphicsWindow() const
	{
		osgViewer::GraphicsWindow* window = dynamic_cast<osgViewer::GraphicsWindow*>(mView->getCamera()->getGraphicsContext());
		assert(window);
		return *window;
	}
};

OsgWindow::OsgWindow(const VisRootPtr& visRoot) :
	mVisRoot(visRoot)
{
	setFlags(Qt::FramelessWindowHint);

	// TODO: we should take the devicePixelRatio() into account

	mWindow = std::make_shared<OsgViewWindow>(createView(width(), height(), std::size_t(winId()), visRoot->getDisplaySettings().vsync));
	mVisRoot->addWindow(mWindow);

	mWindow->getGraphicsWindow().useCursor(true);
	mWindow->getGraphicsWindow().setCursor(osgViewer::GraphicsWindow::MouseCursor::InheritCursor); // Inherit the Qt cursor
}

OsgWindow::~OsgWindow() = default;

skybolt::vis::Window* OsgWindow::getWindow() const
{
	return mWindow.get();
}

void OsgWindow::mousePressEvent(QMouseEvent* event)
{
	emit mousePressed(event->localPos(), event->button(), event->modifiers());
	event->accept();
}

void OsgWindow::mouseReleaseEvent(QMouseEvent* event)
{
	emit mouseReleased(event->localPos(), event->button());
	event->accept();
}

void OsgWindow::mouseMoveEvent(QMouseEvent* event)
{
	mouseMoved(event->localPos(), event->buttons());
	event->accept();
}

void OsgWindow::keyPressEvent(QKeyEvent* event)
{
	event->ignore();
}

bool OsgWindow::event(QEvent* event)
{
    switch (event->type())
    {
    case QEvent::PlatformSurface: {
        auto surfaceEvent = dynamic_cast<QPlatformSurfaceEvent*>(event);
        if (surfaceEvent->surfaceEventType() == QPlatformSurfaceEvent::SurfaceAboutToBeDestroyed)
        {
			// Destroy OSG window since its surface is about to be destroyed
			mVisRoot->removeWindow(mWindow);
			mWindow.reset();
        }
        break;
    }
	case QEvent::TouchBegin: {
		auto touchEvent = dynamic_cast<QTouchEvent*>(event);
		QList<QTouchEvent::TouchPoint> points = touchEvent->touchPoints();
		if (!points.empty())
		{
			QTouchEvent::TouchPoint point = points[0];
			emit mousePressed(point.pos(), Qt::LeftButton, Qt::KeyboardModifiers());
		}
		event->accept();
		break;
	}
	case QEvent::TouchEnd: {
		auto touchEvent = dynamic_cast<QTouchEvent*>(event);
		QList<QTouchEvent::TouchPoint> points = touchEvent->touchPoints();
		if (!points.empty())
		{
			QTouchEvent::TouchPoint point = points[0];
			emit mouseReleased(point.pos(), Qt::LeftButton);
		}
		event->accept();
		break;
	}

	case QEvent::TouchUpdate: {
		auto touchEvent = dynamic_cast<QTouchEvent*>(event);
		QList<QTouchEvent::TouchPoint> points = touchEvent->touchPoints();
		if (!points.empty())
		{
			QTouchEvent::TouchPoint point = points[0];
			emit mouseMoved(point.pos(), Qt::LeftButton);
		}
		event->accept();
		break;
	}

    default:
        break;
    }

    return QWindow::event(event);
}