/* Copyright 2012-2021 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "OsgGeometryHelpers.h"
#include <osg/Drawable>

namespace skybolt {
namespace vis {

void configureDrawable(osg::Drawable& drawable)
{
	drawable.setUseDisplayList(false);
	drawable.setUseVertexBufferObjects(true);
	drawable.setUseVertexArrayObject(true);
}

} // namespace vis
} // namespace skybolt
