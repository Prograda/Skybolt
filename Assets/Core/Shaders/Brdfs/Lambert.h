/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef LAMBERT_H
#define LAMBERT_H
#include "GlobalDefines.h"

float calcLambertDirectionalLight(vec3 L, vec3 N)
{
	return max(0.0f, dot(L, N)) / M_PI;
}

float calcLambertSkyLight(vec3 L, vec3 N)
{
	return (1.0f + dot(L, N)) * 0.5f / M_PI;
}

vec3 calcLambertAmbientLight(vec3 normal, vec3 groundIrradiance, vec3 skyIrradiance)
{
	float groundWeight = (normal.z + 1.0f) * 0.5f;
	return mix(skyIrradiance, groundIrradiance, groundWeight) / M_PI;
}

#endif // LAMBERT_H