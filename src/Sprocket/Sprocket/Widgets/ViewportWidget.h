/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "Sprocket/SprocketFwd.h"
#include "Sprocket/JsonProjectSerializable.h"
#include "Sprocket/ContextAction/ActionContext.h"
#include "Sprocket/Scenario/ScenarioObject.h"
#include "Sprocket/Viewport/SceneObjectPicker.h"
#include "Sprocket/Viewport/ViewportMouseEventHandler.h"

#include <SkyboltEngine/SkyboltEngineFwd.h>
#include <SkyboltEngine/SimVisBinding/EntityVisibilityFilterable.h>
#include <SkyboltSim/World.h>
#include <SkyboltVis/RenderOperation/RenderCameraViewport.h>

#include <boost/signals2/connection.hpp>
#include <filesystem>
#include <QWidget>

class QComboBox;
class QMenu;
class QToolBar;

struct ViewportWidgetConfig
{
	skybolt::EngineRoot* engineRoot;
	skybolt::vis::VisRootPtr visRoot;
	ViewportInputSystemPtr viewportInput;
	SceneSelectionModel* selectionModel;
	std::vector<DefaultContextActionPtr> contextActions;
	std::function<std::string()> projectFilenameGetter;
	QWidget* parent = nullptr;
};

class ViewportWidget : public QWidget, public JsonProjectSerializable, public skybolt::sim::WorldListener
{
	Q_OBJECT
public:
	ViewportWidget(const ViewportWidgetConfig& config);
	~ViewportWidget() override;

	Q_SLOT void update();

	std::optional<PickedSceneObject> pickSceneObjectAtPointInWindow(const QPointF& position, const EntitySelectionPredicate& predicate = &EntitySelectionPredicateAlways) const;
	std::optional<skybolt::sim::Vector3> pickPointOnPlanetAtPointInWindow(const QPointF& position) const;

	//! handlers with lower priority numbers will be executed first
	static constexpr int mouseEventHandlerDefaultPriority = 100;
	void addMouseEventHandler(const ViewportMouseEventHandlerPtr& handler, int priority = mouseEventHandlerDefaultPriority);
	void removeMouseEventHandler(const ViewportMouseEventHandler& handler);

	QMenu* addVisibilityFilterableSubMenu(const QString& text, const skybolt::EntityVisibilityPredicateSetter& setter) const;

	OsgWidget* getViewportCanvas() const { return mOsgWidget; }
	int getViewportWidth() const;
	int getViewportHeight() const;
	glm::dmat4 calcCurrentViewProjTransform() const;

	skybolt::sim::Entity* getCamera() const { return mCurrentSimCamera; }

	QToolBar* getToolBar() const { return mToolBar; }


public: // JsonProjectSerializable
	void resetProject() override;
	void readProject(const nlohmann::json& json) override;
	void writeProject(nlohmann::json& json) const override;

public:
	void entityRemoved(const skybolt::sim::EntityPtr& entity) override;

private:
	QToolBar* createViewportToolBar(const std::function<std::string()>& projectFilenameGetter);

	void captureImage(const std::filesystem::path& baseFilename);

	void setCamera(skybolt::sim::Entity* simCamera);
	void setCameraTarget(skybolt::sim::Entity* target);

	void showContextMenu(const QPoint& point);

private:
	skybolt::EngineRoot* mEngineRoot;
	ViewportInputSystemPtr mViewportInput;
	SceneSelectionModel* mSelectionModel;
	std::vector<DefaultContextActionPtr> mContextActions;
	osg::ref_ptr<skybolt::vis::RenderCameraViewport> mViewport;
	QMenu* mFilterMenu;
	OsgWidget* mOsgWidget;
	QToolBar* mToolBar;
	
	std::multimap<int, ViewportMouseEventHandlerPtr> mMouseEventHandlers; //!< Key is priority, lower values executed first

	QComboBox* mCameraCombo;
	class CameraControllerWidget* mCameraControllerWidget;
	skybolt::sim::Entity* mCurrentSimCamera = nullptr;
	boost::signals2::scoped_connection mViewportCameraConnection;

	SceneObjectPicker mSceneObjectPicker;
};