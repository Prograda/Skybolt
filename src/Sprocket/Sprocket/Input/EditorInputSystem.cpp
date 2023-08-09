/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "EditorInputSystem.h"
#include "InputPlatformOis.h"
#include "Viewport/ViewportInput.h"

using namespace skybolt;

EditorInputSystem::EditorInputSystem(std::shared_ptr<InputPlatform> inputPlatform)
{
	mInputPlatform = std::move(inputPlatform);
	mViewportInput = std::make_unique<ViewportInput>(mInputPlatform);
	mInputPlatform->getEventEmitter()->addEventListener<MouseEvent>(this);
}

void EditorInputSystem::updatePreDynamics(const StepArgs& args)
{
	if (mInputPlatform)
	{
		mViewportInput->updateBeforeInput();
		mInputPlatform->update();
		mViewportInput->updateAfterInput(args.dtWallClock);
	}
}

void EditorInputSystem::onEvent(const Event& event)
{
	if (const auto& mouseEvent = dynamic_cast<const MouseEvent*>(&event))
	{
		if (mouseEvent->type == MouseEvent::Type::Released)
		{
			mInputPlatform->setEnabled(false);
		}
	}
}