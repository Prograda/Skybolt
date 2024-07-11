/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef BLINN_PHONG_H
#define BLINN_PHONG_H

#define M_INV_8PI 0.03978873577

float calcBlinnPhongSpecular(vec3 lightDirection, vec3 viewDirection, vec3 normal, float shininess)
{
	vec3 H = normalize(lightDirection + viewDirection);
    float NdotH = max(dot(normal, H), 0.0);
	
	// This is the normalization factor to use for the half-angle version we are using.
	// See http://www.thetenthplanet.de/archives/255
	float normalizationFactor = (shininess + 8.0) * M_INV_8PI;
	
	return normalizationFactor * pow(NdotH, shininess);
}

#endif // BLINN_PHONG_H