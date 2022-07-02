/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef GROUND_IRRADIANCE_H
#define GROUND_IRRADIANCE_H

#include "GlobalDefines.h"

uniform float groundIrradianceMultiplier;

const vec3 groundAlbedo = vec3(0.3f);

vec3 calcGroundIrradiance(vec3 sunIrradiance, vec3 skyIrradiance, vec3 lightDirection)
{
	return groundIrradianceMultiplier * max(0.f, -lightDirection.z) * groundAlbedo / M_PI * (sunIrradiance + skyIrradiance);
}

#endif // GROUND_IRRADIANCE_H