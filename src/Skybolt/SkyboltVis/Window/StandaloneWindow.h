/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "Window.h"
#include "SkyboltVis/Rect.h"

namespace skybolt {
namespace vis {

struct StandaloneWindowConfig
{
	RectI rect;
};

class StandaloneWindow : public Window
{
public:
	StandaloneWindow(const StandaloneWindowConfig& config);
	StandaloneWindow(const RectI& rect);
	~StandaloneWindow() override = default;

	int getWidth() const override;
	int getHeight() const override;

	std::string getHandle() const;

private:
	osgViewer::GraphicsWindow& getGraphicsWindow() const;
};

} // namespace vis
} // namespace skybolt
