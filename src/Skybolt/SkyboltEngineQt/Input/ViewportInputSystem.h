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