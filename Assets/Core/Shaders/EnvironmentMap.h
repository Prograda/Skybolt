/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef ENVIRONMENT_MAP_H
#define ENVIRONMENT_MAP_H

#include "GlobalDefines.h"

vec2 getSphericalEnvironmentMapTexCoord(vec3 dir)
{
	float theta = atan(dir.y, dir.x) / (2 * M_PI);
	float cosPhi = -dir.z;
	
	return vec2(theta, cosPhi);
}

#endif // ENVIRONMENT_MAP_H