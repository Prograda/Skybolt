#include "ViewportInputSystem.h"
#include <SkyboltEngine/EngineRoot.h>
#include <SkyboltEngine/Input/InputPlatform.h>

using namespace skybolt;

ViewportInputSystem::ViewportInputSystem(const skybolt::InputPlatformPtr& inputPlatform, CameraInputAxes axes, NonNullPtr<EngineRoot> engineRoot) :
	CameraInputSystem(inputPlatform, std::move(axes)),
	mEngineRoot(engineRoot)
{
	assert(mEngineRoot);

	// Start with input disabled. Input will only be enabled on mouse down event in the viewport.
	setMouseEnabled(false);
	setKeyboardEnabled(false);
}

void ViewportInputSystem::onEvent(const Event& event)
{
	if (const auto& mouseEvent = dynamic_cast<const MouseEvent*>(&event))
	{
		// Disable input when mouse is released.
		// Note: enable on mouse down even is handled elsewhere. See OsgWindow::mousePressed.
		if (mouseEvent->type == MouseEvent::Type::Released)
		{
			setMouseEnabled(false);
			setKeyboardEnabled(false);
		}
	}
	CameraInputSystem::onEvent(event);
}

void ViewportInputSystem::setViewportHeight(int heightPixels)
{
	configure(*this, heightPixels, mEngineRoot->engineSettings);
}