/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#version 330 core

in vec3 texCoord;

out vec4 color;

uniform sampler2D heightSampler;
uniform vec2 texelSizeInTextureSpace;
uniform vec2 texelScale;

void main()
{
	vec2 halfTexelSize = 0.5 * texelSizeInTextureSpace;
	float x0 = texture(heightSampler, texCoord.xy + vec2(-halfTexelSize.x, 0)).r;
	float x1 = texture(heightSampler, texCoord.xy + vec2( halfTexelSize.x, 0)).r;
	float y0 = texture(heightSampler, texCoord.xy + vec2(0, -halfTexelSize.y)).r;
	float y1 = texture(heightSampler, texCoord.xy + vec2(0,  halfTexelSize.y)).r;
	
	vec3 tangent = vec3(texelScale.x, 0, x0 - x1);
	vec3 bitangent = vec3(0, texelScale.y, y0 - y1);
	
	color = vec4(normalize(cross(tangent, bitangent)), 1) * 0.5 + 0.5;
}