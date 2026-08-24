/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <SkyboltCommon/File/FileLocator.h>
#include <map>
#include <QIcon>

enum class SkyboltIcon
{
	Add,
	List,
	Node,
	Pause,
	Play,
	Remove,
	SkipBackward,
	SkipForward,
	Speed
};

const QIcon& getSkyboltIcon(SkyboltIcon icon, const skybolt::file::FileLocator& fileLocator);
