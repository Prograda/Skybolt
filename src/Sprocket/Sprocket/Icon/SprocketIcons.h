#pragma once

#include <map>
#include <QIcon>

enum class SprocketIcon
{
	Add,
	Build,
	Code,
	FastForward,
	FastRewind,
	Folder,
	Node,
	NodeGraph,
	Pause,
	Play,
	Remove,
	Save,
	Sequence,
	Settings
};

const QIcon& getSprocketIcon(SprocketIcon icon);
