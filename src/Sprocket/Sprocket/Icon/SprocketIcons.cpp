/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "SprocketIcons.h"
#include "IconFactory.h"

#include <stdexcept>

static std::map<SprocketIcon, QIcon> createSprocketIcons()
{
	std::map<SprocketIcon, std::string> filenames {
		{SprocketIcon::Add, "Icons/google/add_circle.svg"},
		{SprocketIcon::Build, "Icons/google/build.svg"},
		{SprocketIcon::Code, "Icons/google/code.svg"},
		{SprocketIcon::FastForward, "Icons/google/round-fast_forward.svg"},
		{SprocketIcon::FastRewind, "Icons/google/round-fast_rewind.svg"},
		{SprocketIcon::Filter, "Icons/google/filter_alt.svg"},
		{SprocketIcon::Folder, "Icons/google/folder.svg"},
		{SprocketIcon::Node, "Icons/google/brightness_1.svg"},
		{SprocketIcon::NodeGraph, "Icons/google/view_agenda.svg"},
		{SprocketIcon::Pause, "Icons/google/round-pause.svg"},
		{SprocketIcon::Play, "Icons/google/round-play_arrow.svg"},
		{SprocketIcon::Remove, "Icons/google/remove_circle.svg"},
		{SprocketIcon::Save, "Icons/google/save.svg"},
		{SprocketIcon::Screenshot, "Icons/google/screenshot_keyboard.svg"},
		{SprocketIcon::Sequence, "Icons/google/playlist_play.svg"},
		{SprocketIcon::Settings, "Icons/google/settings.svg"},
		{SprocketIcon::Speed, "Icons/google/round-speed.svg"}
	};

	std::map<SprocketIcon, QIcon> icons;
	for (const auto& [icon, filename] : filenames)
	{
		icons[icon] = createIcon(filename).value_or(QIcon());
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