/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef SATURATE_H
#define SATURATE_H

float saturate(float x)
{
	return clamp(x, 0.f, 1.f);
}

vec2 saturate(vec2 x)
{
	return clamp(x, vec2(0), vec2(1));
}

vec3 saturate(vec3 x)
{
	return clamp(x, vec3(0), vec3(1));
}

vec4 saturate(vec4 x)
{
	return clamp(x, vec4(0), vec4(1));
}

#endif // define SATURATE_h