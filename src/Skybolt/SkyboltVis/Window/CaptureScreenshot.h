/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#pragma once

#include "SkyboltVis/SkyboltVisFwd.h"
#include <osg/Image>
#include <string>

namespace skybolt {
namespace vis {

osg::ref_ptr<osg::Image> captureScreenshot(VisRoot& visRoot);
void captureScreenshot(VisRoot& visRoot, const std::string& filename);

} // namespace vis
} // namespace skybolt
