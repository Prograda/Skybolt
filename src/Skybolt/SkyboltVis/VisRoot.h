/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltVis/SkyboltVisFwd.h"
#include "SkyboltVis/DisplaySettings.h"
#include "SkyboltVis/RenderContext.h"

#include <set>

namespace osgViewer {
class CompositeViewer;
class ViewerBase;
}

namespace skybolt {
namespace vis {

class VisRoot
{
public:
	VisRoot(const DisplaySettings& config = DisplaySettings());

	//! @returns false if window has been closed
	bool render();

	osgViewer::ViewerBase& getViewer() const;

	void addWindow(const WindowPtr& window);
	void removeWindow(const WindowPtr& window);

	const std::vector<WindowPtr>& getWindows() const { return mWindows; }

	//! Default is LoadTimingPolicy::LoadAcrossMultipleFrames
	void setLoadTimingPolicy(LoadTimingPolicy loadTimingPolicy) { mLoadTimingPolicy = loadTimingPolicy; }

	const DisplaySettings& getDisplaySettings() const { return mDisplaySettings; }

protected:
	std::shared_ptr<osgViewer::CompositeViewer> mViewer;

private:
	const DisplaySettings mDisplaySettings;
	std::vector<WindowPtr> mWindows;
	LoadTimingPolicy mLoadTimingPolicy;
};

} // namespace vis
} // namespace skybolt
