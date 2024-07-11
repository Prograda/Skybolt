/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

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
