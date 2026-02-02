#include "SkyboltIcons.h"
#include "IconFactory.h"

#include <stdexcept>

static std::map<SkyboltIcon, QIcon> createSkyboltIcons()
{
	std::map<SkyboltIcon, std::string> filenames {
		{SkyboltIcon::Add, "Icons/google/add_circle.svg"},
		{SkyboltIcon::Node, "Icons/google/brightness_1.svg"},
		{SkyboltIcon::Pause, "Icons/google/round-pause.svg"},
		{SkyboltIcon::Play, "Icons/google/round-play_arrow.svg"},
		{SkyboltIcon::Remove, "Icons/google/remove_circle.svg"},
		{SkyboltIcon::SkipBackward, "Icons/google/round-skip-prev.svg"},
		{SkyboltIcon::SkipForward, "Icons/google/round-skip-next.svg"},
		{SkyboltIcon::Speed, "Icons/google/round-speed.svg"}
	};

	std::map<SkyboltIcon, QIcon> icons;
	for (const auto& [icon, filename] : filenames)
	{
		icons[icon] = createIcon(filename).value_or(QIcon());
	}
	return icons;
}

const QIcon& getSkyboltIcon(SkyboltIcon icon)
{
	static std::map<SkyboltIcon, QIcon> icons = createSkyboltIcons();
	auto i = icons.find(icon);
	if (i == icons.end())
	{
		throw std::runtime_error("Icon not found: " + std::to_string((int)icon));
	}
	return i->second;
}