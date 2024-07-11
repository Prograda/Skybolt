/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "WindowUtil.h"

#include <SkyboltVis/Rect.h>
#include <SkyboltVis/VisRoot.h>
#include <osgViewer/CompositeViewer>

namespace skybolt {

std::unique_ptr<vis::VisRoot> createExampleVisRoot()
{
	auto visRoot = std::make_unique<vis::VisRoot>();
	visRoot->getViewer().setKeyEventSetsDone(osgGA::GUIEventAdapter::KEY_Escape);
	return visRoot;
}

} // namespace skybolt
