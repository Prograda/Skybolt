#pragma once

#include <map>
#include <QIcon>

enum class SkyboltIcon
{
	Node,
	Pause,
	Play,
	Sequence,
	SkipBackward,
	SkipForward,
	Speed
};

const QIcon& getSkyboltIcon(SkyboltIcon icon);
