/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

float rerange(float value, float vmin, float vmax)
{
	return (value - vmin) / (vmax - vmin);
}

vec2 rerange(vec2 value, vec2 vmin, vec2 vmax)
{
	return (value - vmin) / (vmax - vmin);
}

vec3 rerange(vec3 value, vec3 vmin, vec3 vmax)
{
	return (value - vmin) / (vmax - vmin);
}

vec4 rerange(vec4 value, vec4 vmin, vec4 vmax)
{
	return (value - vmin) / (vmax - vmin);
}
