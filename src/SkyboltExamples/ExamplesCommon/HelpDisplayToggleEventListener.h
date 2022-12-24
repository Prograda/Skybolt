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
