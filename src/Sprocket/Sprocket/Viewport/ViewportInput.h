#pragma once

#include <SkyboltCommon/Event.h>
#include <SkyboltEngine/SkyboltEngineFwd.h>
#include <SkyboltSim/CameraController/CameraController.h>

class ViewportInput : public skybolt::EventListener
{
public:
	ViewportInput(const skybolt::InputPlatformPtr& inputPlatform);

	~ViewportInput();

	void setEnabled(bool enabled);

	void updateBeforeInput();

	void updateAfterInput(float dt);

	skybolt::sim::CameraController::Input getInput() const;

	void onEvent(const skybolt::Event &evt);

private:
	skybolt::InputPlatformPtr mInputPlatform;
	std::vector<skybolt::LogicalAxisPtr> mLogicalAxes;
	skybolt::LogicalAxisPtr mForwardAxis;
	skybolt::LogicalAxisPtr mRightAxis;
	skybolt::sim::CameraController::Input mInput = skybolt::sim::CameraController::Input::zero();
	bool mEnabled = false;
};
