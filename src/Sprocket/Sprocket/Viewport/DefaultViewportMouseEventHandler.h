/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "ViewportMouseEventHandler.h"
#include "ViewportInput.h"
#include "Sprocket/Scenario/ScenarioObject.h"
#include "Sprocket/SprocketFwd.h"

struct DefaultViewportMouseEventHandlerConfig
{
	ViewportWidget* viewportWidget;
	ViewportInput* viewportInput;
	ScenarioObjectRegistryPtr entityObjectRegistry;
	SceneSelectionModel* sceneSelectionModel;
};

class DefaultViewportMouseEventHandler : public ViewportMouseEventHandler
{
public:
	DefaultViewportMouseEventHandler(DefaultViewportMouseEventHandlerConfig config);
	~DefaultViewportMouseEventHandler() override = default;
	
	void mousePressed(const QPointF& position, Qt::MouseButton button, const Qt::KeyboardModifiers& modifiers) override;
	void mouseReleased(const QPointF& position, Qt::MouseButton button) override;
	void mouseMoved(const QPointF& position, Qt::MouseButtons buttons) override;

protected:
	ViewportWidget* mViewportWidget;
	ViewportInput* mViewportInput;
	ScenarioObjectRegistryPtr mEntityObjectRegistry;
	SceneSelectionModel* mSelectionModel;
};