/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef TONE_MAPPING_H
#define TONE_MAPPING_H

uniform bool convertOutputToSrgb;

vec3 fromLinear(vec3 linearRGB)
{
    bvec3 cutoff = lessThan(linearRGB, vec3(0.0031308));
    vec3 higher = vec3(1.055)*pow(linearRGB, vec3(1.0/2.4)) - vec3(0.055);
    vec3 lower = linearRGB * vec3(12.92);

    return mix(higher, lower, cutoff);
}

vec3 getOutputAsSrgb(vec3 color)
{
	if (convertOutputToSrgb)
		return fromLinear(color);//pow(color, vec3(1.0 / 2.2)); // linear -> sRGB
	else
		return color;
}

const float white_point = 1.0;
const float exposure = 10;

vec3 toneMapWithExposure(vec3 radiance)
{
	vec3 color = vec3(1.0) - exp(-radiance / white_point * exposure);
	return getOutputAsSrgb(color);
}

float brightnessMultiplier = 4.0;

vec3 toneMap(vec3 color)
{
	return getOutputAsSrgb(color * brightnessMultiplier);
}

#endif // TONE_MAPPING_H