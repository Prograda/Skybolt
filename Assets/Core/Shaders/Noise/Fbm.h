/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "Noise/ThirdParty/WebglNoise.h"
#define NUM_OCTAVES 5

// Based on Morgan McGuire @morgan3d
// https://www.shadertoy.com/view/4dS3Wd
float randomMcguire(in vec2 st) {
    return fract(sin(dot(st.xy,
                         vec2(12.9898,78.233)))*
        43758.5453123);
}

float noise(in vec2 st) {
    vec2 i = floor(st);
    vec2 f = fract(st);

    // Four corners in 2D of a tile
    float a = randomMcguire(i);
    float b = randomMcguire(i + vec2(1.0, 0.0));
    float c = randomMcguire(i + vec2(0.0, 1.0));
    float d = randomMcguire(i + vec2(1.0, 1.0));

    vec2 u = f * f * (3.0 - 2.0 * f);

    return mix(a, b, u.x) +
            (c - a)* u.y * (1.0 - u.x) +
            (d - b) * u.x * u.y;
}

float fbm(vec2 p) {
	float total = 0.0, amplitude = 0.3;
	for (int i = 0; i < 8; i++) {
		total += (noise(p)*2-1) * amplitude;
		p = 2.0 * p;
		amplitude *= 0.9;
	}
	return total;
}
