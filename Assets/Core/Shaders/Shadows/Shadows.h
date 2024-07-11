/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef SHADOWS_H
#define SHADOWS_H

#pragma import_defines ( SHADOW_CASCADE_COUNT )
#ifndef SHADOW_CASCADE_COUNT
#define SHADOW_CASCADE_COUNT 1
#endif

#include "Dither.h"
#include "GlobalDefines.h"
#include "Util/Saturate.h"

float sampleShadow(sampler2D shadowSampler, vec2 texCoord, float receiverDepth)
{
	return texture(shadowSampler, texCoord).r > receiverDepth ? 1.0 : 0.0;
}

float sampleShadow(sampler2DShadow shadowSampler, vec2 texCoord, float receiverDepth)
{
	return texture(shadowSampler, vec3(texCoord, receiverDepth));
}

vec2 rotate(vec2 v, float a) {
	float s = sin(a);
	float c = cos(a);
	return vec2(v.x*c - v.y*s, v.x*s + v.y*c);
}

float random(vec2 texCoord)
{
	float dotProduct = dot(texCoord, vec2(12.9898, 78.233));
	return fract(sin(dotProduct) * 43758.5453);
}

// https://docs.microsoft.com/en-us/windows/win32/api/d3d11/ne-d3d11-d3d11_standard_multisample_quality_levels
const vec2 standard4SamplePattern[4] = vec2[4]
(
	vec2(-2.0/8.0, -6.0/8.0),
	vec2(6.0/8.0, -2.0/8.0),
	vec2(-6.0/8.0, 2.0/8.0),
	vec2(2.0/8.0, 6.0/8.0)
);

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

const vec2 poissonDisc16[16] = vec2[]( 
   vec2( -0.94201624, -0.39906216 ), 
   vec2( 0.94558609, -0.76890725 ), 
   vec2( -0.094184101, -0.92938870 ), 
   vec2( 0.34495938, 0.29387760 ), 
   vec2( -0.91588581, 0.45771432 ), 
   vec2( -0.81544232, -0.87912464 ), 
   vec2( -0.38277543, 0.27676845 ), 
   vec2( 0.97484398, 0.75648379 ), 
   vec2( 0.44323325, -0.97511554 ), 
   vec2( 0.53742981, -0.47373420 ), 
   vec2( -0.26496911, -0.41893023 ), 
   vec2( 0.79197514, 0.19090188 ), 
   vec2( -0.24188840, 0.99706507 ), 
   vec2( -0.81409955, 0.91437590 ), 
   vec2( 0.19984126, 0.78641367 ), 
   vec2( 0.14383161, -0.14100790 ) 
);

float sampleShadowPcf4StandardSamplePattern(sampler2DShadow shadowSampler, vec2 texCoord, float receiverDepth)
{
	vec2 offsetScale = 1.0 / textureSize(shadowSampler, 0);
	
	float result = 0.0f;
	const int sampleCount = 4;
	for (int i = 0; i < sampleCount; ++i)
	{
		vec2 offset = standard4SamplePattern[i];
		result += sampleShadow(shadowSampler, texCoord + offset * offsetScale, receiverDepth);
	}

	return result / sampleCount;
}

float sampleShadowPcf3x3(sampler2DShadow shadowSampler, vec2 texCoord, float receiverDepth)
{
	vec2 offsetScale = 1.0 / textureSize(shadowSampler, 0);
	
	float result = 0.0f;
	for (int j = -1; j <= 1; ++j)
	{
		for (int i = -1; i <= 1; ++i)
		{
			vec2 offset = vec2(i, j);
			result += sampleShadow(shadowSampler, texCoord + offset * offsetScale, receiverDepth);
		}
	}

	return result / 9;
}

float sampleShadowPcf8StandardSamplePattern(sampler2DShadow shadowSampler, vec2 texCoord, float receiverDepth)
{
	vec2 offsetScale = 1.0 / textureSize(shadowSampler, 0);
	
	float result = 0.0f;
	const int sampleCount = 8;
	for (int i = 0; i < sampleCount; ++i)
	{
		vec2 offset = standard8SamplePattern[i];
		result += sampleShadow(shadowSampler, texCoord + offset * offsetScale, receiverDepth);
	}

	return result / sampleCount;
}

// 'Optimized PCF' method used in The Witness.
// Based on http://the-witness.net/news/2013/09/shadow-mapping-summary-part-1
// See sample code https://github.com/TheRealMJP/Shadows
// The method weights each PCF sample with a Gaussian curve which gives smoother
// edges for the same number of samples compared with grid-based PCF.
float sampleShadowOptimizedPcf(sampler2DShadow shadowSampler, vec2 texCoord, float receiverDepth)
{
	vec2 offsetScale = vec2(1.0) / textureSize(shadowSampler, 0);
	
	vec2 uv = texCoord * textureSize(shadowSampler, 0);
	vec2 base_uv;
    base_uv.x = floor(uv.x + 0.5);
    base_uv.y = floor(uv.y + 0.5);
	float s = (uv.x + 0.5 - base_uv.x);
    float t = (uv.y + 0.5 - base_uv.y);
	base_uv -= vec2(0.5, 0.5);

	float sum = 0.0f;
	
#define OPTIMIZED_PCF_FILTER_SIZE 5
#if OPTIMIZED_PCF_FILTER_SIZE == 3
	float uw0 = (3 - 2 * s);
	float uw1 = (1 + 2 * s);

	float u0 = (2 - s) / uw0 - 1;
	float u1 = s / uw1 + 1;

	float vw0 = (3 - 2 * t);
	float vw1 = (1 + 2 * t);

	float v0 = (2 - t) / vw0 - 1;
	float v1 = t / vw1 + 1;

	sum += uw0 * vw0 * sampleShadow(shadowSampler, (base_uv + vec2(u0, v0)) * offsetScale, receiverDepth);
	sum += uw1 * vw0 * sampleShadow(shadowSampler, (base_uv + vec2(u1, v0)) * offsetScale, receiverDepth);
	sum += uw0 * vw1 * sampleShadow(shadowSampler, (base_uv + vec2(u0, v1)) * offsetScale, receiverDepth);
	sum += uw1 * vw1 * sampleShadow(shadowSampler, (base_uv + vec2(u1, v1)) * offsetScale, receiverDepth);

	return sum * 1.0f / 16;
#elif OPTIMIZED_PCF_FILTER_SIZE == 5
	float uw0 = (4 - 3 * s);
	float uw1 = 7;
	float uw2 = (1 + 3 * s);

	float u0 = (3 - 2 * s) / uw0 - 2;
	float u1 = (3 + s) / uw1;
	float u2 = s / uw2 + 2;

	float vw0 = (4 - 3 * t);
	float vw1 = 7;
	float vw2 = (1 + 3 * t);

	float v0 = (3 - 2 * t) / vw0 - 2;
	float v1 = (3 + t) / vw1;
	float v2 = t / vw2 + 2;

	sum += uw0 * vw0 * sampleShadow(shadowSampler, (base_uv + vec2(u0, v0)) * offsetScale, receiverDepth);
	sum += uw1 * vw0 * sampleShadow(shadowSampler, (base_uv + vec2(u1, v0)) * offsetScale, receiverDepth);
	sum += uw2 * vw0 * sampleShadow(shadowSampler, (base_uv + vec2(u2, v0)) * offsetScale, receiverDepth);

	sum += uw0 * vw1 * sampleShadow(shadowSampler, (base_uv + vec2(u0, v1)) * offsetScale, receiverDepth);
	sum += uw1 * vw1 * sampleShadow(shadowSampler, (base_uv + vec2(u1, v1)) * offsetScale, receiverDepth);
	sum += uw2 * vw1 * sampleShadow(shadowSampler, (base_uv + vec2(u2, v1)) * offsetScale, receiverDepth);

	sum += uw0 * vw2 * sampleShadow(shadowSampler, (base_uv + vec2(u0, v2)) * offsetScale, receiverDepth);
	sum += uw1 * vw2 * sampleShadow(shadowSampler, (base_uv + vec2(u1, v2)) * offsetScale, receiverDepth);
	sum += uw2 * vw2 * sampleShadow(shadowSampler, (base_uv + vec2(u2, v2)) * offsetScale, receiverDepth);

	return sum * 1.0f / 144;
#elif OPTIMIZED_PCF_FILTER_SIZE == 7
	float uw0 = (5 * s - 6);
	float uw1 = (11 * s - 28);
	float uw2 = -(11 * s + 17);
	float uw3 = -(5 * s + 1);

	float u0 = (4 * s - 5) / uw0 - 3;
	float u1 = (4 * s - 16) / uw1 - 1;
	float u2 = -(7 * s + 5) / uw2 + 1;
	float u3 = -s / uw3 + 3;

	float vw0 = (5 * t - 6);
	float vw1 = (11 * t - 28);
	float vw2 = -(11 * t + 17);
	float vw3 = -(5 * t + 1);

	float v0 = (4 * t - 5) / vw0 - 3;
	float v1 = (4 * t - 16) / vw1 - 1;
	float v2 = -(7 * t + 5) / vw2 + 1;
	float v3 = -t / vw3 + 3;

	sum += uw0 * vw0 * sampleShadow(shadowSampler, (base_uv + vec2(u0, v0)) * offsetScale, receiverDepth);
	sum += uw1 * vw0 * sampleShadow(shadowSampler, (base_uv + vec2(u1, v0)) * offsetScale, receiverDepth);
	sum += uw2 * vw0 * sampleShadow(shadowSampler, (base_uv + vec2(u2, v0)) * offsetScale, receiverDepth);
	sum += uw3 * vw0 * sampleShadow(shadowSampler, (base_uv + vec2(u3, v0)) * offsetScale, receiverDepth);

	sum += uw0 * vw1 * sampleShadow(shadowSampler, (base_uv + vec2(u0, v1)) * offsetScale, receiverDepth);
	sum += uw1 * vw1 * sampleShadow(shadowSampler, (base_uv + vec2(u1, v1)) * offsetScale, receiverDepth);
	sum += uw2 * vw1 * sampleShadow(shadowSampler, (base_uv + vec2(u2, v1)) * offsetScale, receiverDepth);
	sum += uw3 * vw1 * sampleShadow(shadowSampler, (base_uv + vec2(u3, v1)) * offsetScale, receiverDepth);

	sum += uw0 * vw2 * sampleShadow(shadowSampler, (base_uv + vec2(u0, v2)) * offsetScale, receiverDepth);
	sum += uw1 * vw2 * sampleShadow(shadowSampler, (base_uv + vec2(u1, v2)) * offsetScale, receiverDepth);
	sum += uw2 * vw2 * sampleShadow(shadowSampler, (base_uv + vec2(u2, v2)) * offsetScale, receiverDepth);
	sum += uw3 * vw2 * sampleShadow(shadowSampler, (base_uv + vec2(u3, v2)) * offsetScale, receiverDepth);

	sum += uw0 * vw3 * sampleShadow(shadowSampler, (base_uv + vec2(u0, v3)) * offsetScale, receiverDepth);
	sum += uw1 * vw3 * sampleShadow(shadowSampler, (base_uv + vec2(u1, v3)) * offsetScale, receiverDepth);
	sum += uw2 * vw3 * sampleShadow(shadowSampler, (base_uv + vec2(u2, v3)) * offsetScale, receiverDepth);
	sum += uw3 * vw3 * sampleShadow(shadowSampler, (base_uv + vec2(u3, v3)) * offsetScale, receiverDepth);

	return sum * 1.0f / 2704;
#endif
}

float sampleShadowPoisson16(sampler2DShadow shadowSampler, vec2 texCoord, float receiverDepth)
{
	vec2 offsetScale = 3.0 / textureSize(shadowSampler, 0);
	
	float theta = random(texCoord) * M_2PI;
	
	float result = 0.0f;
	const int sampleCount = 16;
	for (int i = 0; i < sampleCount; ++i)
	{
		vec2 offset = rotate(poissonDisc16[i], theta);
		result += sampleShadow(shadowSampler, texCoord + offset * offsetScale, receiverDepth);
	}

	return result / sampleCount;
}

bool inTextureBounds(vec2 texCoord)
{
	vec2 v = abs(texCoord - vec2(0.5));
	return max(v.x, v.y) < 0.5;
}

uniform sampler2DShadow shadowSampler[SHADOW_CASCADE_COUNT];

uniform mat4 shadowProjectionMatrix0;
uniform vec4 cascadeShadowMatrixModifier[SHADOW_CASCADE_COUNT];
uniform vec4 cascadeTexelDepthSizes;
uniform float maxShadowViewDistance;

float calcDepthBias(int cascadeIndex, float dotLN)
{
	float worldTexelSize = cascadeTexelDepthSizes[cascadeIndex];
	
	float dotLNSafe = max(0.1, abs(dotLN));
	
	// Calculate depth error of light space texel projected onto surface.
	// error = worldTexelSize * tan(acos(dotLN))
	// Can be approximated as:
	//float error = worldTexelSize * tan(acos(dotLNSafe));
	float error = -2 * worldTexelSize * (dotLNSafe-1) / dotLNSafe;
	return 0.00005 + error;
}

float sampleShadowCascadesAtTexCoord(vec2 shadowTexcoord, float receiverDepth, float dotLN)
{
	for (int i = 0; i < SHADOW_CASCADE_COUNT; ++i)
	{
		vec2 cascadeTecoord = shadowTexcoord * cascadeShadowMatrixModifier[i].x + cascadeShadowMatrixModifier[i].yz;
		vec2 v = abs(cascadeTecoord - vec2(0.5));
		float d = max(v.x, v.y);

		if (d  < 0.5)
		{
			// Dither-blend to the next cascade at the edges
			float blend = max((0.5-d) * 20, 0.0);
			if (dither(blend) < 0.5)
			{
				// Sample the next cascade
				++i;
				cascadeTecoord = shadowTexcoord * cascadeShadowMatrixModifier[i].x + cascadeShadowMatrixModifier[i].yz;
				if (i == SHADOW_CASCADE_COUNT)
				{
					break;
				}
			}
			
			float cascadeReceiverDepth = receiverDepth + cascadeShadowMatrixModifier[i].w;
			cascadeReceiverDepth -= calcDepthBias(i, dotLN);
			if (i < SHADOW_CASCADE_COUNT-1)
			{
				return sampleShadowPoisson16(shadowSampler[i], cascadeTecoord.xy, cascadeReceiverDepth);
			}
			else
			{
				return sampleShadow(shadowSampler[i], cascadeTecoord.xy, cascadeReceiverDepth);
			}
		}
	}
	return 1.0;
}

float sampleShadowsAtTexCoord(vec2 shadowTexcoord, float receiverDepth, float dotLN, float fragmentViewDistance)
{
	float visibility = sampleShadowCascadesAtTexCoord(shadowTexcoord, receiverDepth, dotLN);
	float blend = saturate((maxShadowViewDistance - fragmentViewDistance) / (maxShadowViewDistance*0.2));
	return mix(1.0, visibility, blend);
}

float sampleShadowsAtWorldPosition(vec3 positionWorldSpace, float dotLN, float fragmentViewDistance)
{
	vec3 shadowTexcoord = (shadowProjectionMatrix0 * vec4(positionWorldSpace, 1)).xyz;
	return sampleShadowsAtTexCoord(shadowTexcoord.xy, shadowTexcoord.z, dotLN, fragmentViewDistance);
}

#endif // SHADOWS_H