/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <SkyboltCommon/NonNullPtr.h>
#include <SkyboltEngine/CameraInputSystem.h>
#include <SkyboltEngine/SkyboltEngineFwd.h>

class ViewportInputSystem : public skybolt::CameraInputSystem
{
public:
	ViewportInputSystem(const skybolt::InputPlatformPtr& inputPlatform, skybolt::CameraInputAxes axes, skybolt::NonNullPtr<skybolt::EngineRoot> engineRoot);

	void setViewportHeight(int heightPixels);

protected:
	void onEvent(const skybolt::Event& event) override;

private:
	skybolt::NonNullPtr<skybolt::EngineRoot> mEngineRoot;
};