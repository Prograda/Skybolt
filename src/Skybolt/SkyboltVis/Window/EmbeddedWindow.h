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

struct EmbeddedWindowConfig
{
	RectI rect;
};

class EmbeddedWindow : public Window
{
public:
	EmbeddedWindow(const EmbeddedWindowConfig& config);

	void setWidth(int width);
	void setHeight(int height);

	virtual int getWidth() const;
	virtual int getHeight() const;

private:
	osgViewer::GraphicsWindowEmbedded* mWindow;
};

} // namespace vis
} // namespace skybolt
