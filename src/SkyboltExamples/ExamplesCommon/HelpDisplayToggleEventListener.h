#pragma once

#include <SkyboltCommon/Event.h>

namespace skybolt {

class HelpDisplaySystem;

class HelpDisplayToggleEventListener : public EventListener
{
public:
	HelpDisplayToggleEventListener(const std::shared_ptr<HelpDisplaySystem>& helpDisplaySystem);

	void onEvent(const Event& event) override;

private:
	std::shared_ptr<HelpDisplaySystem> mHelpDisplaySystem;
};

} // namespace skybolt
