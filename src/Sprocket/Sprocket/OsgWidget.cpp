/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "OsgWidget.h"
#include <SkyboltEngine/Input/InputPlatform.h>

#include <SkyboltVis/Camera.h>
#include <SkyboltVis/Rect.h>
#include <SkyboltVis/Scene.h>
#include <SkyboltVis/RenderTarget/RenderTargetSceneAdapter.h>
#include <SkyboltVis/RenderTarget/Viewport.h>
#include <SkyboltVis/RenderTarget/ViewportHelpers.h>
#include <SkyboltVis/Window/EmbeddedWindow.h>

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
	QOpenGLWidget(parent)
{
	setFocusPolicy(Qt::StrongFocus);

	EmbeddedWindowConfig config;
	config.rect = RectI(0, 0, width(), height());
	config.displaySettings = displaySettings;
	mWindow.reset(new EmbeddedWindow(config));

	auto camera = mWindow->getViewer().getCamera();
	camera->setPreDrawCallback(new DrawCallback(this));
	camera->getOrCreateStateSet()->setDefine("CONVERT_OUTPUT_TO_SRGB");
}

OsgWidget::~OsgWidget()
{
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
	mWindow->render();
}
 
void OsgWidget::resizeGL(int width, int height)
{
	mWindow->setWidth(width);
	mWindow->setHeight(height);
}

void OsgWidget::mousePressEvent(QMouseEvent* event)
{
	if (event->button() == Qt::MouseButton::LeftButton)
	{
		emit mouseDown();
		event->accept();
	}
}

void OsgWidget::keyPressEvent(QKeyEvent* event)
{
	event->ignore();
}
