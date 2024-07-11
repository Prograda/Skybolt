/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "WindowUtil.h"

#include <SkyboltVis/Rect.h>
#include <SkyboltVis/Window/StandaloneWindow.h>
#include <osgViewer/Viewer>

namespace skybolt {

std::unique_ptr<vis::StandaloneWindow> createExampleWindow()
{
	return std::make_unique<vis::StandaloneWindow>(vis::RectI(0, 0, 1080, 720));
}

} // namespace skybolt
