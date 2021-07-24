/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef SPECULAR_H
#define SPECULAR_H

vec3 calcHalfAngle(vec3 V, vec3 L)
{
	return normalize(V + L);
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (vec3(1.0) - F0) * pow(1.0f - cosTheta, 5.0f);
}

// Converts from Blinn-Phong 'shininess' (specular exponent) to Beckmann roughness
// From http://simonstechblog.blogspot.com/2011/12/microfacet-brdf.html
float specularExponentToRoughness(float exponent)
{
	return sqrt(2 / (exponent + 2));
}

float roughnessToSpecularExponent(float roughness)
{
	return 2.0 / (roughness * roughness) - 2;
}

// Glossiness calculation by Dimitar Lazarov, used in Call of Duty: Black Ops 2.
// Used by fresnelSchlickRoughness()
float specularExponentToGlossiness(float exponent)
{
	// https://blog.selfshadow.com/publications/s2013-shading-course/lazarov/s2013_pbs_black_ops_2_slides_v2.pdf
	// Same as log(exponent) / log (8192)
	return log(exponent) / 3.91338994363f;
}

float roughnessToGlossiness(float roughness)
{
	float exponent = roughnessToSpecularExponent(roughness);
	return specularExponentToGlossiness(exponent);
}

// Rough fresnel calculation by Dimitar Lazarov, used in Call of Duty: Black Ops 2.
// @param g is glossiness calculated from specularExponentToGlossiness()
// https://blog.selfshadow.com/publications/s2013-shading-course/lazarov/s2013_pbs_black_ops_2_slides_v2.pdf
vec3 fresnelSchlickRoughness(float dotNV, vec3 F0, float g)
{
	vec4 t = vec4( 1/0.96, 0.475, (0.0275 - 0.25 * 0.04)/0.96, 0.25 );
	t *= vec4( g, g, g, g );
	t += vec4( 0, 0, (0.015 - 0.75 * 0.04)/0.96, 0.75 );
	float a0 = t.x * min( t.y, exp2( -9.28f * dotNV ) ) + t.z;
	float a1 = t.w;
	return clamp( a0 + F0 * ( a1 - a0 ), vec3(0), vec3(1) ).rgb;
}

#endif // define SPECULAR_H