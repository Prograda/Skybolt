/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#version 330 core

in vec3 texCoord;

out vec4 color;

uniform sampler2D displacementSampler;
uniform vec2 texelSizeInTextureSpace;
uniform vec2 textureSizeInWorldSpace;

vec3 sampleVectorDisplacementMap(sampler2D sampler, vec2 texCoord, vec2 textureSizeInWorldSpace)
{
	vec3 P = vec3(texCoord * textureSizeInWorldSpace, 0);
	return P + vec3(0,0,texture(sampler, texCoord).z);
}

void main()
{
	vec2 halfTexelSize = 0.5 * texelSizeInTextureSpace;

	vec3 x0 = sampleVectorDisplacementMap(displacementSampler, texCoord.xy + vec2(-halfTexelSize.x, 0), textureSizeInWorldSpace).xyz;
	vec3 x1 = sampleVectorDisplacementMap(displacementSampler, texCoord.xy + vec2( halfTexelSize.x, 0), textureSizeInWorldSpace).xyz;
	vec3 y0 = sampleVectorDisplacementMap(displacementSampler, texCoord.xy + vec2(0, -halfTexelSize.y), textureSizeInWorldSpace).xyz;
	vec3 y1 = sampleVectorDisplacementMap(displacementSampler, texCoord.xy + vec2(0,  halfTexelSize.y), textureSizeInWorldSpace).xyz;
	
	vec3 tangent = vec3(x1.x - x0.x, 0, x1.z - x0.z);
	vec3 bitangent = vec3(0, y1.y - y0.y, y1.z - y0.z);
	
	color = vec4(normalize(cross(tangent, bitangent)), 1) * 0.5 + 0.5;
}
