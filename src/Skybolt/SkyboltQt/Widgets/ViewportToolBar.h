/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltQt/SkyboltQtFwd.h"
#include <SkyboltEngine/SkyboltEngineFwd.h>
#include <QPointer>
#include <QToolBar>

class QComboBox;

struct ViewportToolBarConfig
{
	skybolt::EngineRoot* engineRoot;
	std::function<std::string()> scenarioFilenameGetter;
	QPointer<ViewportWidget> viewport;
	QWidget* parent = nullptr;
};

class ViewportToolBar : public QToolBar
{
public:
	ViewportToolBar(ViewportToolBarConfig config);

	QMenu* getVisibilityFilterMenu() const { return mFilterMenu; }

private:
	QMenu* mFilterMenu;
	CameraControllerWidget* mCameraControllerWidget;
	QComboBox* mCameraCombo;
};
