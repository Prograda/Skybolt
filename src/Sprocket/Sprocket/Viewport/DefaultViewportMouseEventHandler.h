/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "ViewportMouseEventHandler.h"
#include "Sprocket/Scenario/ScenarioObject.h"
#include "Sprocket/SprocketFwd.h"

struct DefaultViewportMouseEventHandlerConfig
{
	ViewportWidget* viewportWidget;
	ViewportInputSystemPtr viewportInput;
	ScenarioObjectRegistryPtr entityObjectRegistry;
	SceneSelectionModel* sceneSelectionModel;
};

class DefaultViewportMouseEventHandler : public ViewportMouseEventHandler
{
public:
	DefaultViewportMouseEventHandler(DefaultViewportMouseEventHandlerConfig config);
	~DefaultViewportMouseEventHandler() override = default;
	
	bool mousePressed(const QPointF& position, Qt::MouseButton button, const Qt::KeyboardModifiers& modifiers) override;
	bool mouseReleased(const QPointF& position, Qt::MouseButton button) override;
	bool mouseMoved(const QPointF& position, Qt::MouseButtons buttons) override;

protected:
	virtual void selectItems(const std::vector<ScenarioObjectPtr>& objects);

protected:
	ViewportWidget* mViewportWidget;
	ViewportInputSystemPtr mViewportInput;
	ScenarioObjectRegistryPtr mEntityObjectRegistry;
	SceneSelectionModel* mSelectionModel;
};