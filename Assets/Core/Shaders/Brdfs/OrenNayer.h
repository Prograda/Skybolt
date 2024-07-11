/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef OREN_NAYER_H
#define OREN_NAYER_H
#include "GlobalDefines.h"

// Oren Nayer approximation from 'Beyond a Simple Physically Based Blinn-Phong Model in Real-Time' by Yoshiharu Gotanda.
// https://blog.selfshadow.com/publications/s2012-shading-course/gotanda/s2012_pbs_beyond_blinn_slides_v3.pdf
float calcOrenNayerDirectionalLight(vec3 lightDirection, vec3 viewDirection, vec3 normal, float roughness)
{
	float r2 = roughness * roughness;
	float dotNL = dot(normal, lightDirection);
	float dotEL = dot(viewDirection, lightDirection);
	float dotNE = dot(normal, viewDirection);
	
	float A = dotNL * (1.0 - 0.5 * r2 / (r2 + 0.33));
	float B = 0.45 * (r2 / (r2 + 0.09)) * max(0, dotEL - dotNE * dotNL) * min(1, dotNL / dotNE);
	
	return max(0, (A + B)) / M_PI;
}

#endif // OREN_NAYER_H
