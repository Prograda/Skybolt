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
	OsgWidget(const skybolt::vis::DisplaySettings& displaySettings, QWidget* parent = 0);
	virtual ~OsgWidget();

	skybolt::vis::Window* getWindow() const;

signals:
	void mouseDown();

protected:
	virtual void initializeGL();
	virtual void resizeGL(int width, int height);
	virtual void paintGL();

	virtual void mousePressEvent(QMouseEvent* event);
	virtual void keyPressEvent(QKeyEvent* event);

private:
	std::unique_ptr<skybolt::vis::EmbeddedWindow> mWindow;
	osg::ref_ptr<osg::Camera::DrawCallback> mDrawCallback;
};
