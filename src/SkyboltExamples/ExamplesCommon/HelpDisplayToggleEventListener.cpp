#include "HelpDisplayToggleEventListener.h"

#include <SkyboltEngine/Input/InputPlatform.h>

#include <assert.h>

namespace skybolt {

HelpDisplayToggleEventListener::HelpDisplayToggleEventListener(const osg::ref_ptr<HelpDisplayRenderOperation>& helpDisplay) :
	mHelpDisplay(helpDisplay)
{
	assert(mHelpDisplay);
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
					mHelpDisplay->setVisible(!mHelpDisplay->isVisible());
					break;
				}
			}
		}
	}
}

} // namespace skybolt
