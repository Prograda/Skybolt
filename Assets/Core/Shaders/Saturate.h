/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

float saturate(float value)
{
	return clamp(value, 0, 1);
}

vec2 saturate(vec2 value)
{
	return clamp(value, vec2(0), vec2(1));
}

vec3 saturate(vec3 value)
{
	return clamp(value, vec3(0), vec3(1));
}

vec4 saturate(vec4 value)
{
	return clamp(value, vec4(0), vec4(1));
}
