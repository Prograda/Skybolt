/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef GLOBALUNIFORMS_H
#define GLOBALUNIFORMS_H

uniform vec3 planetCenter;
uniform mat4 planetMatrixInv;
uniform float innerRadius;

uniform vec3 wrappedNoiseOrigin;
uniform float wrappedNoisePeriod;

vec3 calcWrappedNoiseCoord(vec3 position)
{
	return (position - wrappedNoiseOrigin) / wrappedNoisePeriod;
}

#endif // GLOBALUNIFORMS_H
