/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "SkyboltIcons.h"
#include "IconFactory.h"

#include <stdexcept>

static std::map<SkyboltIcon, QIcon> createSkyboltIcons()
{
	std::map<SkyboltIcon, std::string> filenames {
		{SkyboltIcon::Add, "Icons/google/add_circle.svg"},
		{SkyboltIcon::Build, "Icons/google/build.svg"},
		{SkyboltIcon::Code, "Icons/google/code.svg"},
		{SkyboltIcon::FastForward, "Icons/google/round-fast_forward.svg"},
		{SkyboltIcon::FastRewind, "Icons/google/round-fast_rewind.svg"},
		{SkyboltIcon::Filter, "Icons/google/filter_alt.svg"},
		{SkyboltIcon::Folder, "Icons/google/folder.svg"},
		{SkyboltIcon::Node, "Icons/google/brightness_1.svg"},
		{SkyboltIcon::NodeGraph, "Icons/google/view_agenda.svg"},
		{SkyboltIcon::Pause, "Icons/google/round-pause.svg"},
		{SkyboltIcon::Play, "Icons/google/round-play_arrow.svg"},
		{SkyboltIcon::Remove, "Icons/google/remove_circle.svg"},
		{SkyboltIcon::Save, "Icons/google/save.svg"},
		{SkyboltIcon::Screenshot, "Icons/google/screenshot_keyboard.svg"},
		{SkyboltIcon::Sequence, "Icons/google/playlist_play.svg"},
		{SkyboltIcon::Settings, "Icons/google/settings.svg"},
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