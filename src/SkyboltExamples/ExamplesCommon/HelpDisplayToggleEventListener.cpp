#include "HelpDisplayToggleEventListener.h"
#include "HelpDisplaySystem.h"

#include <SkyboltEngine/Input/InputPlatform.h>

#include <assert.h>

namespace skybolt {

HelpDisplayToggleEventListener::HelpDisplayToggleEventListener(const std::shared_ptr<HelpDisplaySystem>& helpDisplaySystem) :
	mHelpDisplaySystem(helpDisplaySystem)
{
	assert(mHelpDisplaySystem);
}

void HelpDisplayToggleEventListener::onEvent(const Event& event)
{
	if (const KeyEvent* keyEvent = dynamic_cast<const KeyEvent*>(&event))
	{
		if (keyEvent->type == KeyEvent::Pressed)
		{
			switch (keyEvent->code)
			{
				case KC_H:
				{
					mHelpDisplaySystem->setVisible(!mHelpDisplaySystem->isVisible());
					break;
				}
			}
		}
	}
}

} // namespace skybolt
