/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SprocketFwd.h"
#include "TreeWidget/WorldTreeWidget.h"
#include "Viewport/SceneObjectPicker.h"
#include <SkyboltEngine/EngineRoot.h>
#include <SkyboltEngine/Plugin/PluginHelpers.h>
#include <SkyboltSim/SkyboltSimFwd.h>
#include <SkyboltCommon/Event.h>
#include <QMainWindow>
#include <QSettings>
#include <ToolWindowManager/ToolWindowManager.h>

#include <cxxtimer/cxxtimer.hpp>

class SprocketModel;
class OsgWidget;
class QAction;
class QComboBox;
class QModelIndex;
class PropertyEditor;
class TreeItem;
class WorldTreeWidget;

namespace Ui { class MainWindow; }
namespace skybolt { class VisEntityIcons; }

class Application;

class MainWindow : public QMainWindow, public skybolt::EventListener
{
	Q_OBJECT

public:
	MainWindow(const std::vector<skybolt::PluginFactory>& enginePluginFactories, const std::vector<EditorPluginFactory>& pluginFactories, QWidget *parent = 0, Qt::WindowFlags flags = Qt::WindowFlags());
	~MainWindow();

	skybolt::EngineRoot* getEngineRoot() const { return mEngineRoot.get(); }

	void clearProject();
	void open(const QString& filename);
	void save(class QFile& file);

	void addToolWindow(const QString& windowName, QWidget* window);
	void raiseToolWindow(QWidget* widget);

	QMenu* addVisibilityFilterableSubMenu(const QString& text, skybolt::EntityVisibilityFilterable* filterable) const;

	std::optional<PickedSceneObject> pickSceneObjectAtPointInWindow(const QPointF& position, const EntitySelectionPredicate& predicate = &EntitySelectionPredicateAlways) const;
	std::optional<skybolt::sim::Vector3> pickPointOnPlanetAtPointInWindow(const QPointF& position) const;

	using ViewportClickHandler = std::function<void(Qt::MouseButton button, const QPointF& position)>;
	ViewportClickHandler getDefaultViewportClickHandler();
	void setViewportClickHandler(ViewportClickHandler handler) { mViewportClickHandler = std::move(handler); }

public slots:
	void newScenario();

private slots:
	void newEntityTemplate();
	void open();
	void save();
	void saveAs();
	void updateIfIntervalElapsed();
	void about();
	void exit();
	void captureImage();
	void editEngineSettings();
	void setViewportTextureDisplayEnabled(bool enabled);
	void setLiveShaderEditingEnabled(bool enabled);

	void toolWindowActionToggled(bool state);
	void toolWindowVisibilityChanged(QWidget* toolWindow, bool visible);
	
	void explorerSelectionChanged(const TreeItem& item);

	void onViewportMouseDown(Qt::MouseButton button, const QPointF& position);

	void showContextMenu(const QPoint& point);

protected:
	virtual void loadProject(const QJsonObject& json);
	virtual void saveProject(QJsonObject& json) const;

	virtual void update();

	virtual void setSelectedEntity(std::weak_ptr<skybolt::sim::Entity> entity);
	virtual void setPropertiesModel(PropertiesModelPtr properties);

private:
	void onEvent(const skybolt::Event& event) override;

private:
	void loadViewport(const QJsonObject& json);
	QJsonObject saveViewport() const;

	void setCamera(const skybolt::sim::EntityPtr& simCamera);
	QToolBar * createViewportToolBar();
	QString getDefaultProjectDirectory() const;
	void closeEvent(QCloseEvent*);

	void setProjectFilename(const QString& filename);

	void addViewportMenuActions();

	void setCameraTarget(skybolt::sim::Entity* target);

	glm::dmat4 calcCurrentViewProjTransform() const;

private:
	std::unique_ptr<skybolt::EngineRoot> mEngineRoot;
	std::unique_ptr<SprocketModel> mSprocketModel;
	std::unique_ptr<skybolt::sim::SimStepper> mSimStepper;
	std::unique_ptr<Ui::MainWindow> ui;
	QAction* mViewMenuToolWindowSeparator;
	ToolWindowManager* mToolWindowManager;
	std::vector<QAction*> mToolActions;
	std::unique_ptr<skybolt::vis::ShaderSourceFileChangeMonitor> mShaderSourceFileChangeMonitor;
	std::shared_ptr<skybolt::StatsDisplaySystem> mStatsDisplaySystem;
	cxxtimer::Timer mUpdateTimer; //!< Time since last update

	class EntitiesTableModel* mEntitiesTableModel;
	class PictureTableModel* mPictureTableModel;
	class PlanTableModel* mPlanTableModel;
	std::shared_ptr<class PropertiesModel> mPropertiesModel;

	std::vector<DefaultContextActionPtr> mContextActions;

	bool mDisableInputSystemOnNextUpdate = false;
	std::shared_ptr<skybolt::InputPlatform> mInputPlatform;
	std::unique_ptr<class ViewportInput> mViewportInput;

	PropertyEditor* mPropertiesEditor = nullptr;
	QComboBox* mCameraCombo;
	class CameraControllerWidget* mCameraControllerWidget;

	QString mProjectFilename;

	SceneObjectPicker mSceneObjectPicker;

	osg::ref_ptr<skybolt::VisEntityIcons> mVisSelectedEntityIcon;
	std::unique_ptr<skybolt::VisNameLabels> mVisNameLabels;
	std::unique_ptr<skybolt::VisOrbits> mVisOrbits;
	std::unique_ptr<skybolt::ForcesVisBinding> mForcesVisBinding;
	skybolt::sim::EntityPtr mCurrentSimCamera;

	OsgWidget* mOsgWidget = nullptr;
	WorldTreeWidget* mWorldTreeWidget = nullptr;

	QSettings mSettings;
	nlohmann::json mEngineSettings;
	std::unique_ptr<class RecentFilesMenuPopulator> mRecentFilesMenuPopulator;

	std::vector<EditorPluginPtr> mPlugins;

	osg::ref_ptr<skybolt::vis::RenderCameraViewport> mViewport;
	ViewportClickHandler mViewportClickHandler = getDefaultViewportClickHandler();
	osg::ref_ptr<skybolt::vis::RenderOperation> mRenderOperationVisualization;

	std::weak_ptr<skybolt::sim::Entity> mSelectedEntity;
};
