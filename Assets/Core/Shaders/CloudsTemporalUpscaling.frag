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
uniform mat4 reprojectionMatrix;
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
	float logZNdc = texture(depthTextureSrc, texCoord.xy).r;
		
	int bayerValue = int(bayerIndex[int(gl_FragCoord.x) % 4][int(gl_FragCoord.y) % 4]);
	if (bayerValue == (frameNumber % 16))
	{
		// Use new data
		colorOut = srcColor;
	}
	else
	{
		// No new data available. Reproject previous frame's data.
	
		// Calculate min and max color of 3x3 box around source sample point
		// based on https://www.elopezr.com/temporal-aa-and-the-quest-for-the-holy-trail/

		// Calculate sample point in previous frame
		float clipSpaceW = calcClipSpaceWFromLogZNdc(logZNdc);

		vec4 pos = reprojectionMatrix * vec4((texCoord.xy * 2.0 - 1.0) * clipSpaceW, -clipSpaceW, 1.0);
		vec2 prevTexCoord = pos.xy / -pos.z * 0.5 + 0.5;

		if (prevTexCoord.x < 0 || prevTexCoord.x > 1 || prevTexCoord.y < 0 || prevTexCoord.y > 1)
		{
			// Reseacle texture coodinates to account for texture dimensions non divisible by 4
			vec2 texCoordRescale = 0.25 * textureSize(colorTexturePrev, 0) / textureSize(colorTextureSrc, 0);
			//vec2 texCoordRescale = 0.25 * textureDims / vec2(int(textureDims.x / 4), int(textureDims.y / 4));
		
			// Sample source texture at coordinate corresponding screen coordinate
			colorOut = texture(colorTextureSrc, texCoord.xy * texCoordRescale - jitterOffset);
		}
		else
		{
			vec4 minColor = srcColor;
			vec4 maxColor = srcColor;
			
			for (int i = 0; i < neighborClampingTapCount; ++i)
			{
				ivec2 neighborTexCoord = texCoordInt + neighborClampingOffsets[i];
				
				vec4 color = texelFetch(colorTextureSrc, texCoordInt + neighborClampingOffsets[i], 0);
				minColor = min(minColor, color); // Take min and max
				maxColor = max(maxColor, color);
			}
		
			vec4 prevColor = sampleTextureCatmullRom(colorTexturePrev, prevTexCoord.xy);
			prevColor = clamp(prevColor, minColor, maxColor);
		
			colorOut = prevColor;
		}
	}
	depthOut = logZNdc;
}
