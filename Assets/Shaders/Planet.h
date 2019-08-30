/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "GlobalUniforms.h"

uniform vec2 latLonMin;
uniform vec2 latLonMax;

vec3 getPlanetSurfacePoint(vec2 uv)
{
	float lat = -mix(latLonMin.x, latLonMax.x, uv.y);
	float lon = mix(latLonMin.y, latLonMax.y, uv.x);
	
	float cosLat = cos(lat);
	return vec3(cos(lon) * cosLat, -sin(lon) * cosLat, sin(lat)) * innerRadius;
}

// Returns drop of curved planet's surface from a plane
// Taylor series expansion of R(1-cos(d/R))
float planetSurfaceDrop(float distance)
{
	return distance*distance / (2 * innerRadius);
}
