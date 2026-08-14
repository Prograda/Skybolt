/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "OsgWindow.h"
#include <SkyboltVis/VisRoot.h>
#include <SkyboltVis/Window/StandaloneWindow.h>

// This must be included before GraphicsWindowX11
#include <QKeyEvent>

#include <osgViewer/ViewerBase>

#ifdef Q_OS_WIN
#	include <osgViewer/api/win32/GraphicsWindowWin32>
#elif defined(Q_OS_LINUX)
#	include <osgViewer/api/X11/GraphicsWindowX11>
#else
#	error "Operating system not supported"
#endif

using namespace skybolt;
using namespace skybolt::vis;

static osg::ref_ptr<osg::Referenced> createWindowData(WId hwnd)
{
#ifdef Q_OS_WIN
	return new osgViewer::GraphicsWindowWin32::WindowData(HWND(hwnd));
#elif defined(Q_OS_LINUX)
	auto platform = QGuiApplication::platformName();
	if (platform == "xcb")
	{
		return new osgViewer::GraphicsWindowX11::WindowData(hwnd);
	}
#endif
	qFatal("SkyboltWEngineQt::OsgWindow - unsupported platform: '%s'", qPrintable(QGuiApplication::platformName()));
	return {};
}

static osg::ref_ptr<osgViewer::View> createView(int width, int height, osg::ref_ptr<osg::Referenced> windowData, bool vsync)
{
	if (!windowData)
	{
		qFatal() << "Invalid windowData provided";
	}
	osg::ref_ptr<osg::GraphicsContext::Traits> traits(new osg::GraphicsContext::Traits);
	traits->x = 0;
	traits->y = 0;
	traits->width = width;
	traits->height = height;
	traits->red = 8;
	traits->green = 8;
	traits->blue = 8;
	//traits->alpha = 8; // Intentially left unset because Qt window does not support alpha by default
	traits->depth = 24;
	traits->windowDecoration = false;
	traits->doubleBuffer = true;
	traits->inheritedWindowData = windowData;

	// FIXME: There's a bug in OSG where vsync is left at OS default when vsync=false, not actually set to false.
	// See https://github.com/openscenegraph/OpenSceneGraph/blob/master/src/osgViewer/GraphicsWindowWin32.cpp#L1978
	traits->vsync = vsync;

	traits->readDISPLAY();
	traits->setUndefinedScreenDetailsToDefaultScreen();

	osg::ref_ptr<osg::GraphicsContext> context = osg::GraphicsContext::createGraphicsContext(traits.get());
	if (!context) 
	{
		qFatal() << "Failed to initialise osg::GraphicsContext";
	}

	configureGraphicsState(*context);
	
	osg::ref_ptr<osgViewer::View> view = new osgViewer::View;
	view->getCamera()->setViewport(new osg::Viewport(0, 0, width, height));
	view->getCamera()->setGraphicsContext(context);
	return view;
}

class OsgViewWindow : public skybolt::vis::Window
{
public:
	OsgViewWindow(const osg::ref_ptr<osgViewer::View>& view) :
		skybolt::vis::Window(view)
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

	show(); // On X11 we must show the window before creating the window, otherwise window appears empty.

	auto window = std::make_shared<OsgViewWindow>(createView(width(), height(), createWindowData(winId()), visRoot->getDisplaySettings().vsync));
	window->getGraphicsWindow().useCursor(true);
	window->getGraphicsWindow().setCursor(osgViewer::GraphicsWindow::MouseCursor::InheritCursor); // Inherit the Qt cursor
	mWindow = window;

	mVisRoot->addWindow(mWindow);
}

OsgWindow::~OsgWindow()
{
	// Qt might destroy OsgWindow without sending a SurfaceAboutToBeDestroyed event, so we need to remove the OSG window here as well.
	if (mWindow)
	{
		mVisRoot->removeWindow(mWindow);
	}
}

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
#ifdef Q_OS_LINUX
	// The QEvent::Resize event needs specific handling on linux.
	case QEvent::Resize: {
        QResizeEvent* resizeEvent = static_cast<QResizeEvent*>(event);
        
		if (auto osgViewWindow = dynamic_cast<OsgViewWindow*>(mWindow.get()); osgViewWindow)
		{
			osgViewer::GraphicsWindow& graphicsWindow = osgViewWindow->getGraphicsWindow();
			if (auto x11Window = dynamic_cast<osgViewer::GraphicsWindowX11*>(&graphicsWindow); x11Window)
			{
				int width = resizeEvent->size().width();
				int height = resizeEvent->size().height();
				x11Window->resized(0, 0, width, height);
				x11Window->getEventQueue()->windowResize(0, 0, width, height);
			}
        }
		break;
    }
#endif
    default:
        break;
    }

    return QWindow::event(event);
}