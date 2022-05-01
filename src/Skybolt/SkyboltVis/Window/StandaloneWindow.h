/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "DisplaySettings.h"
#include "Window.h"
#include "SkyboltVis/Rect.h"

namespace skybolt {
namespace vis {

struct StandaloneWindowConfig
{
	RectI rect;
	DisplaySettings displaySettings;
};

class StandaloneWindow : public Window
{
public:
	StandaloneWindow(const StandaloneWindowConfig& config);
	StandaloneWindow(const RectI& rect);

	virtual int getWidth() const;
	virtual int getHeight() const;

	std::string getHandle() const;
};

} // namespace vis
} // namespace skybolt
