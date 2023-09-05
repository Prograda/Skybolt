/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "DefaultViewportMouseEventHandler.h"
#include "Sprocket/Input/ViewportInputSystem.h"
#include "Sprocket/Scenario/ScenarioSelectionModel.h"
#include "Sprocket/Widgets/ViewportWidget.h"

#include <SkyboltEngine/EngineRoot.h>
#include <SkyboltSim/Components/NameComponent.h>
#include <SkyboltSim/Components/CameraControllerComponent.h>

using namespace skybolt;

DefaultViewportMouseEventHandler::DefaultViewportMouseEventHandler(DefaultViewportMouseEventHandlerConfig config) :
	mEngineRoot(config.engineRoot),
	mViewportInput(config.viewportInput),
	mSelectionModel(config.scenarioSelectionModel),
	mSelectionPredicate(config.selectionPredicate)
{
	assert(mEngineRoot);
	assert(mViewportInput);
	assert(mSelectionModel);
	assert(mSelectionPredicate);
}

bool DefaultViewportMouseEventHandler::mousePressed(ViewportWidget& widget, const QPointF& position, Qt::MouseButton button, const Qt::KeyboardModifiers& modifiers)
{
	if (button == Qt::MouseButton::LeftButton)
	{
		if (mViewportInput)
		{
			mViewportInput->setKeyboardEnabled(true);
			configure(*mViewportInput, widget.getViewportHeight(), mEngineRoot->engineSettings);

			if (sim::Entity* camera = widget.getCamera(); camera)
			{
				mViewportCameraConnection = mViewportInput->cameraInputGenerated.connect([this, cameraId = camera->getId()] (const skybolt::sim::CameraController::Input& input) {
					if (sim::Entity* camera = mEngineRoot->scenario->world.getEntityById(cameraId).get(); camera)
					{
						if (auto controller = camera->getFirstComponent<sim::CameraControllerComponent>(); controller)
						{
							if (controller->getSelectedController())
							{
								controller->getSelectedController()->setInput(input);
							}
						}
					}
				});
			}

			return true;
		}
	}
	return false;
}

bool DefaultViewportMouseEventHandler::mouseReleased(ViewportWidget& widget, const QPointF& position, Qt::MouseButton button)
{
	if (mViewportInput)
	{
		mViewportInput->setKeyboardEnabled(false);
		mViewportInput->setMouseEnabled(false);
	}

	if (button == Qt::MouseButton::LeftButton)
	{
		sim::EntityId selectedEntityId = sim::nullEntityId();
			
		std::optional<PickedScenarioObject> pickedObject = widget.pickSceneObjectAtPointInWindow(position, mSelectionPredicate);
		selectItems(pickedObject ? SelectedScenarioObjects({pickedObject->object}) : SelectedScenarioObjects());
		return true;
	}
	return false;
}

bool DefaultViewportMouseEventHandler::mouseMoved(ViewportWidget& widget, const QPointF& position, Qt::MouseButtons buttons)
{
	if (buttons & Qt::LeftButton)
	{
		if (mViewportInput)
		{
			mViewportInput->setMouseEnabled(true);
			return true;
		}
	}
	return false;
}

void DefaultViewportMouseEventHandler::selectItems(const std::vector<ScenarioObjectPtr>& objects)
{
	mSelectionModel->setSelectedItems(objects);
}