/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "Window.h"
#include "SkyboltVis/Rect.h"

namespace skybolt {
namespace vis {

struct OffscreenWindowConfig
{
	int width;
	int height;
};

class OffscreenWindow : public Window
{
public:
	OffscreenWindow(const OffscreenWindowConfig& config);
	OffscreenWindow(int width, int height);

	virtual int getWidth() const;
	virtual int getHeight() const;

private:
	int mWidth;
	int mHeight;
};

} // namespace vis
} // namespace skybolt
