/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef SHADOWS_H
#define SHADOWS_H

float sampleShadow(sampler2D shadowSampler, vec2 texCoord, float receiverDepth)
{
	return texture(shadowSampler, texCoord).r > receiverDepth ? 1.0 : 0.0;
}

float sampleShadowPcf4(sampler2D shadowSampler, vec2 texCoord, float receiverDepth)
{
	vec2 offsetScale = 1.0 / textureSize(shadowSampler, 0);

	return (
		sampleShadow(shadowSampler, texCoord + offsetScale * vec2(-2.0/8.0, -6.0/8.0), receiverDepth) +
		sampleShadow(shadowSampler, texCoord + offsetScale * vec2(6.0/8.0, -2.0/8.0), receiverDepth) +
		sampleShadow(shadowSampler, texCoord + offsetScale * vec2(-6.0/8.0, 2.0/8.0), receiverDepth) +
		sampleShadow(shadowSampler, texCoord + offsetScale * vec2(2.0/8.0, 6.0/8.0), receiverDepth)
		) * 0.25;
}

// https://docs.microsoft.com/en-us/windows/win32/api/d3d11/ne-d3d11-d3d11_standard_multisample_quality_levels
const vec2 standard8SamplePattern[8] = vec2[8]
(
	vec2(1.0/8.0, -3.0/8.0),
	vec2(-1.0/8.0, 3.0/8.0),
	vec2(5.0 / 8.0, 1.0 / 8.0),
	vec2(-3.0 / 8.0, -5.0 / 8.0),
	vec2(-5.0 / 8.0, 5.0 / 8.0),
	vec2(-7.0 / 8.0, -1.0 / 8.0),
	vec2(3.0 / 8.0, 7.0 / 8.0),
	vec2(7.0 / 8.0, -7.0 / 8.0)
	);

float sampleShadowPcf8(sampler2D shadowSampler, vec2 texCoord, float receiverDepth)
{
	vec2 offsetScale = 1.0 / textureSize(shadowSampler, 0);

	float result = 0.0f;
	for (int i = 0; i < 8; ++i)
	{
		result += sampleShadow(shadowSampler, texCoord + offsetScale * standard8SamplePattern[i], receiverDepth);
	}

	return result / 8.0f;
}

bool inTextureBounds(vec2 texCoord)
{
	vec2 v = abs(texCoord - vec2(0.5));
	return max(v.x, v.y) < 0.5;
}

uniform sampler2D shadowSampler0;
uniform mat4 shadowProjectionMatrix;

float sampleShadowsAtTexCoord(vec2 shadowTexcoord, float receiverDepth)
{
	vec2 v = abs(shadowTexcoord - vec2(0.5));
	float d = max(max(v.x, v.y), receiverDepth - 0.5);
	
	if (d  < 0.5)
	{	
		float weight = max(0.0, d * 4.0 - 1.0);
		return mix(sampleShadowPcf8(shadowSampler0, shadowTexcoord.xy, receiverDepth), 1.0, weight);
	}
	return 1.0;
}

float sampleShadowsAtWorldPosition(vec3 positionWorldSpace)
{
	vec2 shadowTexcoord = (shadowProjectionMatrix * vec4(positionWorldSpace, 1)).xy;
	float receiverDepth = 0.5; // TODO
	return sampleShadowsAtTexCoord(shadowTexcoord, receiverDepth);
}

#endif // SHADOWS_H