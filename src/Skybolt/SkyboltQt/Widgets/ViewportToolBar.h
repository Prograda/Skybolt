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
