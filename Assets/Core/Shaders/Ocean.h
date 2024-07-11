/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef OCEAN_H
#define OCEAN_H

#define NUM_OCEAN_CASCADES 1

const vec3 shallowScatterColor = vec3(0.16, 0.2, 0.25);
const vec3 deepScatterColor = vec3(0.04, 0.15, 0.47) * 0.098; // From Bruenton ocean demo

// Specular values calibrated from photos of Earth https://petapixel.com/2021/09/30/breathtaking-photos-of-earth-were-taken-from-even-higher-than-the-iss/
const float oceanShininess = 150;
const vec3 oceanSpecularColor = vec3(0.02);

const float oceanMeshFadeoutStartDistance = 10000;
const float oceanMeshFadeoutEndDistance = 40000;

const float airIor = 1.0;
const float waterIor = 1.333;
const float sqrtR0 = (airIor - waterIor) / (airIor + waterIor);
const float R0 = sqrtR0 * sqrtR0;

// Schlick fresnel model
float calcSchlickFresnel(float dotVN)
{
	float cosTheta = abs(dotVN);
	return R0 + (1.0 - R0) * pow(1.0 - cosTheta, 5.0);
}

// Fresnel from https://www.shadertoy.com/view/Ms2SD1
// TODO: determine if this is a good cheep approximation or not
float calcHackFresnel(float dotVN)
{
    float fresnel = clamp(1.0 - dotVN, 0.0, 1.0);
    return pow(fresnel, 3.0) * 0.5;
}

#endif // OCEAN_H