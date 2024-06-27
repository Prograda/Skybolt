/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <map>
#include <QIcon>

enum class SkyboltIcon
{
	Add,
	Build,
	Code,
	FastForward,
	FastRewind,
	Filter,
	Folder,
	Node,
	NodeGraph,
	Pause,
	Play,
	Remove,
	Save,
	Screenshot,
	Sequence,
	Settings,
	Speed
};

const QIcon& getSkyboltIcon(SkyboltIcon icon);
