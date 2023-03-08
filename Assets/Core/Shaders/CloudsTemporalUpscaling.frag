/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#version 330 core

#include "DepthPrecision.h"
#include "Util/SampleTextureCatmullRom.h"

in vec3 texCoord;
uniform sampler2D colorTextureSrc;
uniform sampler2D depthTextureSrc;
uniform sampler2D colorTexturePrev;
uniform sampler2D depthTexturePrev;
uniform sampler2D sceneDepthSampler;

uniform mat4 reprojectionMatrix;
uniform mat4 invReprojectionMatrix;

uniform int frameNumber;
uniform vec2 jitterOffset;

layout(location = 0) out vec4 colorOut;
layout(location = 1) out float depthOut;

const mat4 bayerIndex = mat4(
    vec4(00.0, 12.0, 03.0, 15.0),
    vec4(08.0, 04.0, 11.0, 07.0),
    vec4(02.0, 14.0, 01.0, 13.0),
    vec4(10.0, 06.0, 09.0, 05.0));

const int neighborClampingTapCount = 8;
const ivec2 neighborClampingOffsets[neighborClampingTapCount] = ivec2[neighborClampingTapCount](
	ivec2(-1,-1), ivec2(0,-1), ivec2(1,-1),
	ivec2(-1, 0),              ivec2(1, 0),
	ivec2(-1, 1), ivec2(0, 1), ivec2(1, 1)
);

void main()
{
	ivec2 texCoordInt = ivec2(gl_FragCoord.x/4, gl_FragCoord.y/4);
	vec4 srcColor = texelFetch(colorTextureSrc, texCoordInt, 0);
	float logZNdc = texelFetch(depthTextureSrc, texCoordInt, 0).r;
	
	float sceneLogZNdc = texelFetch(sceneDepthSampler, ivec2(gl_FragCoord.xy), 0).r;
		
	int bayerValue = int(bayerIndex[int(gl_FragCoord.x) % 4][int(gl_FragCoord.y) % 4]);
	if (bayerValue == (frameNumber % 16)) // if current pixel was sampled this frame
	{
		// Used sample directly
		colorOut = srcColor;
		//colorOut = vec4(1,1,1,1);
	}
	else // current pixel was not sampled this frame
	{
		// Reconstrct sample
		
		float clipSpaceW = calcClipSpaceWFromLogZNdc(logZNdc);

		vec4 pos = reprojectionMatrix * vec4((texCoord.xy * 2.0 - 1.0) * clipSpaceW, -clipSpaceW, 1.0);
		vec2 prevTexCoord = pos.xy / -pos.z * 0.5 + 0.5;

		// Read depth from previous frame
		// Use a texelFetch because depth values do not interpolate meaningfully at the edges
		float logZNdcPrev = texelFetch(depthTexturePrev, ivec2(prevTexCoord.xy * textureSize(depthTexturePrev, 0)), 0).r;

		if (prevTexCoord.x < 0 || prevTexCoord.x > 1 || prevTexCoord.y < 0 || prevTexCoord.y > 1
			|| logZNdcPrev < 0) // if offscreen previous frame
		{
			// Rescale texture coodinates to account for texture dimensions non divisible by 4
			vec2 texCoordRescale = 0.25 * textureSize(colorTexturePrev, 0) / textureSize(colorTextureSrc, 0);
		
			// Sample source texture at coordinate corresponding screen coordinate
			colorOut = textureLod(colorTextureSrc, texCoord.xy * texCoordRescale - jitterOffset, 0);
		}
		else // sample point is on screen in previous frame
		{
			// Read color from previous frame
			vec4 prevColor = sampleTextureCatmullRom(colorTexturePrev, prevTexCoord.xy);

			// Reproject depth value into current frame
			float clipSpaceW = calcClipSpaceWFromLogZNdc(logZNdcPrev);
			vec4 pos = invReprojectionMatrix * vec4((prevTexCoord.xy * 2.0 - 1.0) * clipSpaceW, -clipSpaceW, 1.0);
			logZNdc = calcLogZNdc(-pos.z);

			// Suppress ghosting artifacts
			// Calculate min and max color of 3x3 box around source sample point
			// based on https://www.elopezr.com/temporal-aa-and-the-quest-for-the-holy-trail/

			vec4 minColor = srcColor;
			vec4 maxColor = srcColor;
			
			for (int i = 0; i < neighborClampingTapCount; ++i)
			{
				ivec2 neighborTexCoord = texCoordInt + neighborClampingOffsets[i];
				
				vec4 color = texelFetch(colorTextureSrc, texCoordInt + neighborClampingOffsets[i], 0);
				
				minColor = min(minColor, color);
				maxColor = max(maxColor, color);
			}

			vec4 clampedPrevColor = clamp(prevColor, minColor, maxColor);

			// Clamping suppressed ghosting, but causes some buzzing.
			// Since ghosting only occurs when the camera moves, we scale clamping weight
			// with camera movement to avoid buzzing for stationary or slow camera movement.
			float blendWeight = min(1.0, length(prevTexCoord-texCoord.xy)*200);
			colorOut = mix(prevColor, clampedPrevColor, blendWeight);
		}
	}
	
	if (sceneLogZNdc < 0.7 && sceneLogZNdc < logZNdc) // if clouds are occluded
	{
		colorOut = vec4(0);
		depthOut = -1;
		return;
	}
	depthOut = logZNdc;
}
