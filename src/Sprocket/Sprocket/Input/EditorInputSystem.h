/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "Sprocket/SprocketFwd.h"

#include <SkyboltCommon/Event.h>
#include <SkyboltEngine/SkyboltEngineFwd.h>
#include <SkyboltSim/System/System.h>

class EditorInputSystem : public skybolt::sim::System, public skybolt::EventListener
{
public:
	EditorInputSystem(std::shared_ptr<skybolt::InputPlatform> inputPlatform);

	ViewportInput* getViewportInput() const { return mViewportInput.get(); }

public: // skybolt::sim::System
	void updatePreDynamics(const StepArgs& args) override;

private: // skybolt::EventListener interface
	void onEvent(const skybolt::Event& event) override;

private:
	std::shared_ptr<skybolt::InputPlatform> mInputPlatform;
	std::unique_ptr<ViewportInput> mViewportInput;
};