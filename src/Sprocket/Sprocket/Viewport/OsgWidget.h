/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <SkyboltEngine/SkyboltEngineFwd.h>
#include <SkyboltVis/SkyboltVisFwd.h>

#include <QOpenGLWidget>
#include <osg/Camera>

#define CONVERT_TO_SRGB_IN_SHADER // TODO: get rid of this when Qt implements this for windows (currently only implemented for XCB)

class OsgWidget : public QOpenGLWidget
{
	Q_OBJECT
public:
	OsgWidget(const skybolt::vis::VisRootPtr& visRoot, QWidget* parent = 0);
	~OsgWidget() override;

	skybolt::vis::VisRoot* getVisRoot() const;
	skybolt::vis::Window* getWindow() const;

signals:
	void mousePressed(const QPointF& position, Qt::MouseButton button, const Qt::KeyboardModifiers& modifiers);
	void mouseReleased(const QPointF& position, Qt::MouseButton button);
	void mouseMoved(const QPointF& position, Qt::MouseButtons buttons);

protected:
	void initializeGL() override;
	void resizeGL(int width, int height) override;
	void paintGL() override;

	void mousePressEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void keyPressEvent(QKeyEvent* event) override;

private:
	skybolt::vis::VisRootPtr mVisRoot;
	std::shared_ptr<skybolt::vis::EmbeddedWindow> mWindow;
	osg::ref_ptr<osg::Camera::DrawCallback> mDrawCallback;
};
