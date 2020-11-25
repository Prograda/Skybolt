/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef CLOUDS_H
#define CLOUDS_H

#include "GlobalDefines.h"

uniform vec2 cloudsDisplacementMeters;

vec2 offsetCloudUvByWorldDistance(vec2 startUv, vec2 worldDistance)
{
	return startUv + vec2(worldDistance.y / 40070000.0, 0.5 * worldDistance.x / 40070000.0);
}

vec2 uvFromPos(vec3 positionRelPlanetPlanetAxes)
{
	vec2 pos2d = normalize(positionRelPlanetPlanetAxes.yx);
	float psi = atan(pos2d.x, pos2d.y);
	float theta = asin(normalize(positionRelPlanetPlanetAxes).z);
	vec2 uv = vec2(psi * M_RCP_2PI + 0.5, theta * M_RCP_PI + 0.5);
	return offsetCloudUvByWorldDistance(uv, -cloudsDisplacementMeters);
}

#endif // CLOUDS_H