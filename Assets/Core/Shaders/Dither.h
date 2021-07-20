/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef DITHER_H
#define DITHER_H

float dither(float c)
{
	// Dither technique from https://www.shadertoy.com/view/MslGR8
	vec3 magic = vec3(0.06711056, 0.00583715, 52.9829189);
	return floor(c + fract(magic.z * fract(dot(gl_FragCoord.xy, magic.xy))));
}

#endif // DITHER_H