/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef LAMBERT_H
#define LAMBERT_H
#include "GlobalDefines.h"

uniform float groundIrradianceMultiplier;

float calcLambertDirectionalLight(vec3 L, vec3 N)
{
	return max(0.0f, dot(L, N)) / M_PI;
}

float calcLambertSkyLight(vec3 L, vec3 N)
{
	return (1.0f + dot(L, N)) * 0.5f / M_PI;
}

const vec3 groundAlbedo = vec3(0.3f);

vec3 calcGroundIrradiance(vec3 sunIrradiance, vec3 skyIrradiance, vec3 lightDirection)
{
	return groundIrradianceMultiplier * max(0.0, -lightDirection.z) * groundAlbedo / M_PI * (sunIrradiance + skyIrradiance);
}

vec3 calcLambertAmbientLight(vec3 normal, vec3 groundIrradiance, vec3 skyIrradiance)
{
	float groundWeight = (normal.z + 1.0f) * 0.5f;
	return mix(skyIrradiance, groundIrradiance, groundWeight) / M_PI;
}

#endif // LAMBERT_H