/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "OsgMathHelpers.h"

namespace skybolt {
namespace math {

void getOrthonormalBasis(const osg::Vec3f &normal, osg::Vec3f &tangent, osg::Vec3f &bitangent)
{
	float d = normal * osg::Vec3f(0, 1, 0);
	if (d > -0.95f && d < 0.95f)
		bitangent = normal ^ osg::Vec3f(0, 1, 0);
	else
		bitangent = normal ^ osg::Vec3f(0, 0, 1);
	bitangent.normalize();
	tangent = bitangent ^ normal;
}

} // namespace math
} // namespace skybolt
