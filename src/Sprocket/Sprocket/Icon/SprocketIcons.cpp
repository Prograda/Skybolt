#include "SprocketIcons.h"
#include "IconFactory.h"

static std::map<SprocketIcon, QIcon> createSprocketIcons()
{
	std::map<SprocketIcon, std::string> filenames {
		{SprocketIcon::Add, "Icons/google/add_circle.svg"},
		{SprocketIcon::Build, "Icons/google/build.svg"},
		{SprocketIcon::Code, "Icons/google/code.svg"},
		{SprocketIcon::FastForward, "Icons/google/round-fast_forward.svg"},
		{SprocketIcon::FastRewind, "Icons/google/round-fast_rewind.svg"},
		{SprocketIcon::Folder, "Icons/google/folder.svg"},
		{SprocketIcon::Node, "Icons/google/brightness_1.svg"},
		{SprocketIcon::NodeGraph, "Icons/google/view_agenda.svg"},
		{SprocketIcon::Pause, "Icons/google/round-pause.svg"},
		{SprocketIcon::Play, "Icons/google/round-play_arrow.svg"},
		{SprocketIcon::Remove, "Icons/google/remove_circle.svg"},
		{SprocketIcon::Save, "Icons/google/save.svg"},
		{SprocketIcon::Sequence, "Icons/google/playlist_play.svg"},
		{SprocketIcon::Settings, "Icons/google/settings.svg"}
	};

	std::map<SprocketIcon, QIcon> icons;
	for (const auto& [icon, filename] : filenames)
	{
		if (auto i = createIcon(filename); i)
		{
			icons[icon] = *i;
		}
	}
	return icons;
}

const QIcon& getSprocketIcon(SprocketIcon icon)
{
	static std::map<SprocketIcon, QIcon> icons = createSprocketIcons();
	auto i = icons.find(icon);
	if (i == icons.end())
	{
		throw std::runtime_error("Icon not found: " + std::to_string((int)icon));
	}
	return i->second;
}