/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SprocketFwd.h"
#include "WorldTreeWidget.h"
#include <SkyboltEngine/EngineRoot.h>
#include <SkyboltEngine/Plugin/PluginHelpers.h>
#include <SkyboltSim/SkyboltSimFwd.h>
#include <SkyboltCommon/Event.h>
#include <QMainWindow>
#include <qtoolwindowmanager.h>

#include <boost/timer.hpp>
#include <time.h>

class SprocketModel;
class OsgWidget;
class QAction;
class QComboBox;
class QModelIndex;
class QwtPlot;
class PropertyEditor;
class TreeItem;
class WorldTreeWidget;

namespace Ui { class MainWindow; }

class Application;

class MainWindow : public QMainWindow, public skybolt::EventListener
{
	Q_OBJECT

public:
	MainWindow(const std::vector<skybolt::PluginFactory>& enginePluginFactories, const std::vector<EditorPluginFactory>& pluginFactories, QWidget *parent = 0, Qt::WindowFlags flags = 0);
	~MainWindow();

	skybolt::EngineRoot* getEngineRoot() const { return mEngineRoot.get(); }

	void open(const QString& filename);

public slots:
	void newScenario();

private slots:
	void newEntityTemplate();
	void open();
	void save();
	void saveAs();
	void update();
	void about();
	void exit();
	void captureImage();
	void editEngineSettings();
	void setLiveShaderEditingEnabled(bool enabled);

	void toolWindowActionToggled(bool state);
	void toolWindowVisibilityChanged(QWidget* toolWindow, bool visible);
	
	void explorerSelectionChanged(const TreeItem& item);

	void enableViewportInput();

private:
	void onEvent(const skybolt::Event& event) override;

private:
	void loadViewport(const QJsonObject& json);
	QJsonObject saveViewport();

	void setCamera(const skybolt::sim::EntityPtr& simCamera);
	QToolBar * createViewportToolBar();
	QString getDefaultProjectDirectory() const;
	void clearProject();
	void save(class QFile& file);
	void closeEvent(QCloseEvent*);
	void addToolWindow(const QString& windowName, QWidget* window);
	void raiseToolWindow(QWidget* widget);

	void setProjectFilename(const QString& filename);

	std::vector<TreeItemContextActionPtr> createContextActions() const;

	QMenu* addVisibilityFilterableSubMenu(QMenu& parent, const QString& text, skybolt::EntityVisibilityFilterable* filterable) const;

	void addViewportMenuActions(QMenu& menu);

	void setCameraTarget(skybolt::sim::Entity* target);

private:
	std::unique_ptr<skybolt::EngineRoot> mEngineRoot;
	std::unique_ptr<SprocketModel> mSprocketModel;
	std::unique_ptr<skybolt::sim::SimStepper> mSimStepper;
	std::unique_ptr<Ui::MainWindow> ui;
	QToolWindowManager* mToolWindowManager;
	std::vector<QAction*> mToolActions;
	std::unique_ptr<skybolt::vis::ShaderSourceFileChangeMonitor> mShaderSourceFileChangeMonitor;
	std::shared_ptr<skybolt::StatsDisplaySystem> mStatsDisplaySystem;

	class EntitiesTableModel* mEntitiesTableModel;
	class PictureTableModel* mPictureTableModel;
	class PlanTableModel* mPlanTableModel;
	std::shared_ptr<class PropertiesModel> mPropertiesModel;

	bool mDisableInputSystemOnNextUpdate = false;
	std::shared_ptr<skybolt::InputPlatform> mInputPlatform;
	std::unique_ptr<class ViewportInput> mViewportInput;

	skybolt::vis::ArrowsPtr mArrows;

	PropertyEditor* mPropertiesEditor = nullptr;
	QComboBox* mCameraCombo;
	class CameraControllerWidget* mCameraControllerWidget;

	QString mProjectFilename;

	std::unique_ptr<skybolt::VisNameLabels> mVisNameLabels;
	std::unique_ptr<skybolt::VisOrbits> mVisOrbits;
	std::unique_ptr<skybolt::ForcesVisBinding> mForcesVisBinding;
	skybolt::sim::EntityPtr mCurrentSimCamera;

	QwtPlot* mPlot = nullptr;
	OsgWidget* mOsgWidget = nullptr;
	WorldTreeWidget* mWorldTreeWidget = nullptr;

	QSettings mSettings;
	nlohmann::json mEngineSettings;
	std::unique_ptr<class RecentFilesMenuPopulator> mRecentFilesMenuPopulator;

	std::vector<EditorPluginPtr> mPlugins;

	osg::ref_ptr<skybolt::vis::RenderTarget> mRenderTarget;

	skybolt::sim::Entity* mSelectedEntity = nullptr;
};
