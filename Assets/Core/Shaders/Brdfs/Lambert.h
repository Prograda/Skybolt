/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef LAMBERT_H
#define LAMBERT_H
#include "GlobalDefines.h"

float calcLambertDirectionalLight(vec3 L, vec3 N)
{
	return max(0.0, dot(L, N)) / M_PI;
}

float calcLambertSkyLight(vec3 L, vec3 N)
{
	return (1.0 + dot(L, N)) * 0.5 / M_PI;
}

const vec3 groundAlbedo = vec3(0.3);
const vec3 normalizedGroundReflectance = groundAlbedo / M_PI;

vec3 calcLambertAmbientLight(vec3 normal, vec3 sunIrradiance, vec3 skyIrradiance)
{
	vec3 groundReflectance = normalizedGroundReflectance * (sunIrradiance + skyIrradiance);
	float groundWeight = (normal.z + 1.0) * 0.5;
	return mix(skyIrradiance, groundReflectance, groundWeight) / M_PI;
}

#endif // LAMBERT_H