/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "Noise/ThirdParty/WebglNoise.h"
#define NUM_OCTAVES 5

float fbm(vec3 x, vec3 period)
{
    float v = 0.0f;
    float amplitude = 1.0;
    float amplitudeSum = 0.0;
	vec3 shift = vec3(100);
    for(int i = 0; i < NUM_OCTAVES; ++i)
    {
        v += pnoise(x, period) * amplitude;
        amplitudeSum += amplitude;
        amplitude *= 0.5;
		x = x * 2.0 + shift;
		period *= 2.0;
    }
	return v / amplitudeSum;
    //return clamp(v, 0, 1);
}

// @param normalizedX must have a range of [0, 1) and can repeat
float scaledFbm(vec3 normalizedX, float scale)
{
	return fbm(normalizedX * scale, vec3(scale));
}