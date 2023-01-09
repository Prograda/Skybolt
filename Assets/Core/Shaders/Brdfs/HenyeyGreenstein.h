/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef HENYEY_GREENSTEIN_H
#define HENYEY_GREENSTEIN_H
 
const float oneOnFourPi = 0.0795774715459;

vec4 henyeyGreenstein(float cosAngle, vec4 eccentricity)
{
    vec4 eccentricity2 = eccentricity*eccentricity;
    return oneOnFourPi * ((vec4(1.0) - eccentricity2) / pow(vec4(1.0) + eccentricity2 - 2.0*eccentricity*cosAngle, vec4(3.0/2.0)));
}

float henyeyGreenstein(float cosAngle, float eccentricity)
{
	return henyeyGreenstein(cosAngle, vec4(eccentricity)).x;
}

float watooHenyeyGreenstein(float cosAngle)
{
	// Tuned to match The "Watoo" phase function for clouds, from Bouthors et al.
	// See http://wiki.nuaj.net/index.php?title=Clouds
	vec4 phaseG = vec4(-0.2, 0.3, 0.96, 0);
	vec4 weights = vec4(0.5, 0.5, 0.03, 0);
	return dot(henyeyGreenstein(cosAngle, phaseG), weights);
}

#endif // HENYEY_GREENSTEIN_H