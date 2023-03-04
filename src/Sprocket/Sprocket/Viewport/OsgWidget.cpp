/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "OsgWidget.h"
#include <SkyboltVis/VisRoot.h>
#include <SkyboltVis/Window/EmbeddedWindow.h>

#include <osgViewer/ViewerBase>
#include <QKeyEvent>

using namespace skybolt;
using namespace skybolt::vis;

class DrawCallback : public osg::Camera::DrawCallback
{
public:
	DrawCallback(OsgWidget* widget) : widget(widget) {}
	void operator() (const osg::Camera &) const
	{
		// Makes the context current and binds Qt's frame buffer
		widget->makeCurrent();
	}

	OsgWidget* widget;
};

OsgWidget::OsgWidget(const DisplaySettings& displaySettings, QWidget* parent):
	QOpenGLWidget(parent),
	mVisRoot(std::make_unique<VisRoot>(displaySettings))
{
	setFocusPolicy(Qt::StrongFocus);

	// Since Qt manages the OpenGL context, ensure OSG renders on the Qt thread
	mVisRoot->getViewer().setThreadingModel(osgViewer::ViewerBase::SingleThreaded);

	EmbeddedWindowConfig config;
	config.rect = RectI(0, 0, width(), height());
	mWindow = std::make_shared<EmbeddedWindow>(config);
	mVisRoot->addWindow(mWindow);

	auto camera = mWindow->getView()->getCamera();
	camera->setPreDrawCallback(new DrawCallback(this));
	camera->getOrCreateStateSet()->setDefine("CONVERT_OUTPUT_TO_SRGB");
}

OsgWidget::~OsgWidget()
{
}

skybolt::vis::VisRoot* OsgWidget::getVisRoot() const
{
	return mVisRoot.get();
}

skybolt::vis::Window* OsgWidget::getWindow() const
{
	return mWindow.get();
}

void OsgWidget::initializeGL()
{
}
 
void OsgWidget::paintGL()
{
	mVisRoot->render();
}
 
void OsgWidget::resizeGL(int width, int height)
{
	mWindow->setWidth(width);
	mWindow->setHeight(height);
}

void OsgWidget::mousePressEvent(QMouseEvent* event)
{
	emit mouseDown(event->button(), event->localPos());
	event->accept();
}

void OsgWidget::keyPressEvent(QKeyEvent* event)
{
	event->ignore();
}
