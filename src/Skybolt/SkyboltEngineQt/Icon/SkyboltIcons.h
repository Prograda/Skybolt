#pragma once

#include <map>
#include <QIcon>

enum class SkyboltIcon
{
	Add,
	Node,
	Pause,
	Play,
	Remove,
	SkipBackward,
	SkipForward,
	Speed
};

const QIcon& getSkyboltIcon(SkyboltIcon icon);
