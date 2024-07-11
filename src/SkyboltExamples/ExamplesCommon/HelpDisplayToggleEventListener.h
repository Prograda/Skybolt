/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "HelpDisplayRenderOperation.h"
#include <SkyboltCommon/Event.h>

namespace skybolt {

class HelpDisplayToggleEventListener : public EventListener
{
public:
	HelpDisplayToggleEventListener(const osg::ref_ptr<HelpDisplayRenderOperation>& helpDisplaySystem);

	void onEvent(const Event& event) override;

private:
	osg::ref_ptr<HelpDisplayRenderOperation> mHelpDisplay;
};

} // namespace skybolt
