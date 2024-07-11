/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef REMAP_H
#define REMAP_H

float remap(float originalValue, float originalMin, float originalMax, float newMin, float newMax)
{
	return newMin + (((originalValue - originalMin) / (originalMax - originalMin)) * (newMax - newMin));
}

vec2 remap(vec2 originalValue, vec2 originalMin, vec2 originalMax, vec2 newMin, vec2 newMax)
{
	return newMin + (((originalValue - originalMin) / (originalMax - originalMin)) * (newMax - newMin));
}

float remapNormalized(float originalValue, float originalMin, float originalMax)
{
	return (originalValue - originalMin) / (originalMax - originalMin);
}

vec2 remapNormalized(vec2 originalValue, vec2 originalMin, vec2 originalMax)
{
	return (originalValue - originalMin) / (originalMax - originalMin);
}

vec3 remapNormalized(vec3 originalValue, vec3 originalMin, vec3 originalMax)
{
	return (originalValue - originalMin) / (originalMax - originalMin);
}

vec4 remapNormalized(vec4 originalValue, vec4 originalMin, vec4 originalMax)
{
	return (originalValue - originalMin) / (originalMax - originalMin);
}

#endif // REMAP_H