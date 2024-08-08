/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltQt/SkyboltQtFwd.h"
#include "SkyboltQt/ContextAction/ActionContext.h"
#include "SkyboltQt/Scenario/JsonScenarioSerializable.h"
#include "SkyboltQt/Scenario/ScenarioObject.h"
#include "SkyboltQt/Viewport/ScenarioObjectPicker.h"
#include "SkyboltQt/Viewport/ViewportMouseEventHandler.h"

#include <SkyboltEngine/SkyboltEngineFwd.h>
#include <SkyboltEngine/SimVisBinding/EntityVisibilityFilterable.h>
#include <SkyboltSim/World.h>
#include <SkyboltVis/RenderOperation/RenderCameraViewport.h>

#include <filesystem>
#include <QWidget>

class QComboBox;
class QMenu;

struct ViewportWidgetConfig
{
	QString viewportName = "DefaultViewport";
	skybolt::EngineRoot* engineRoot;
	skybolt::vis::VisRootPtr visRoot;
	ScenarioSelectionModel* selectionModel;
	ScenarioObjectPicker scenarioObjectPicker;
	std::vector<DefaultContextActionPtr> contextActions;
	QWidget* parent = nullptr;
};

class ViewportWidget : public QWidget, public JsonScenarioSerializable, public skybolt::sim::WorldListener
{
	Q_OBJECT
public:
	ViewportWidget(const ViewportWidgetConfig& config);
	~ViewportWidget() override;

	void setCamera(skybolt::sim::Entity* simCamera);

	void captureImage(const std::filesystem::path& baseFilename);

	Q_SLOT void update();

	std::vector<PickedScenarioObject> pickSceneObjectsAtPointInWindow(const QPointF& position, const ScenarioObjectPredicate& predicate = &ScenarioObjectPredicateAlways) const;
	std::optional<skybolt::sim::Vector3> pickPointOnPlanetAtPointInWindow(const QPointF& position) const;

	//! handlers with lower priority numbers will be executed first
	static constexpr int mouseEventHandlerDefaultPriority = 100;
	void addMouseEventHandler(const ViewportMouseEventHandlerPtr& handler, int priority = mouseEventHandlerDefaultPriority);
	void removeMouseEventHandler(const ViewportMouseEventHandler& handler);

	QWidget* getViewportCanvas() const { return mOsgWidget; }
	int getViewportWidth() const;
	int getViewportHeight() const;
	glm::dmat4 calcCurrentViewProjTransform() const;

	skybolt::sim::Entity* getCamera() const { return mCurrentSimCamera; } //!< May return null

	osg::ref_ptr<skybolt::vis::RenderCameraViewport> getRenderCameraViewport() const { return mViewport; }

public: // JsonScenarioSerializable
	void resetScenario() override;
	void readScenario(const nlohmann::json& json) override;
	void writeScenario(nlohmann::json& json) const override;

public:
	void entityRemoved(const skybolt::sim::EntityPtr& entity) override;

private:
	void showContextMenu(const QPoint& point);

private:
	skybolt::EngineRoot* mEngineRoot;
	skybolt::vis::VisRootPtr mVisRoot;
	ViewportInputSystemPtr mViewportInput;
	ScenarioSelectionModel* mSelectionModel;
	std::vector<DefaultContextActionPtr> mContextActions;
	osg::ref_ptr<skybolt::vis::RenderCameraViewport> mViewport;
	OsgWindow* mOsgWindow;
	QWidget* mOsgWidget;
	
	std::multimap<int, ViewportMouseEventHandlerPtr> mMouseEventHandlers; //!< Key is priority, lower values executed first

	QComboBox* mCameraCombo;
	skybolt::sim::Entity* mCurrentSimCamera = nullptr;

	ScenarioObjectPicker mScenarioObjectPicker;
};
