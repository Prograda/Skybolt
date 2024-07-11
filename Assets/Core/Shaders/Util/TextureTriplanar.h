/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef TEXTURE_TRIPLANAR_H
#define TEXTURE_TRIPLANAR_H

vec4 textureTriplanar(sampler2D sampler, vec3 position, vec3 normal)
{
	float tighten = 0.4679f; 
	vec3 weights = clamp(abs(normal) - tighten, 0, 1);
	
	float total = weights.x + weights.y + weights.z;
	weights /= total;
	
	vec4 cXY = texture(sampler, position.xy);
	vec4 cXZ = texture(sampler, position.xz);
	vec4 cYZ = texture(sampler, position.yz);	
	
	return weights.z * cXY + weights.y * cXZ + weights.x * cYZ; 
}

#endif TEXTURE_TRIPLANAR_H