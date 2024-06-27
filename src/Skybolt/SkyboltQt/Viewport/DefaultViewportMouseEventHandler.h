/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "ViewportMouseEventHandler.h"
#include "SkyboltQt/Scenario/ScenarioObject.h"
#include "SkyboltQt/Scenario/ScenarioObjectPredicate.h"
#include "SkyboltQt/SkyboltQtFwd.h"
#include <SkyboltEngine/SkyboltEngineFwd.h>

#include <boost/signals2/connection.hpp>

struct DefaultViewportMouseEventHandlerConfig
{
	skybolt::EngineRoot* engineRoot;
	ViewportInputSystemPtr viewportInput;
	ScenarioSelectionModel* scenarioSelectionModel;
	std::function<ScenarioObjectPredicate(const ViewportWidget&)> viewportSelectionPredicate;
};

class DefaultViewportMouseEventHandler : public ViewportMouseEventHandler
{
public:
	DefaultViewportMouseEventHandler(DefaultViewportMouseEventHandlerConfig config);
	~DefaultViewportMouseEventHandler() override = default;
	
	bool mousePressed(ViewportWidget& widget, const QPointF& position, Qt::MouseButton button, const Qt::KeyboardModifiers& modifiers) override;
	bool mouseReleased(ViewportWidget& widget, const QPointF& position, Qt::MouseButton button) override;
	bool mouseMoved(ViewportWidget& widget, const QPointF& position, Qt::MouseButtons buttons) override;

protected:
	virtual void selectItems(const std::vector<ScenarioObjectPtr>& objects);

protected:
	skybolt::EngineRoot* mEngineRoot;
	ViewportInputSystemPtr mViewportInput;
	ScenarioSelectionModel* mSelectionModel;
	std::function<ScenarioObjectPredicate(const ViewportWidget&)> mViewportSelectionPredicate;

	boost::signals2::scoped_connection mViewportCameraConnection;
};