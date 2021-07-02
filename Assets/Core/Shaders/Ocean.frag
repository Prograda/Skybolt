/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#version 440 core
#pragma import_defines ( DISTANCE_CULL )
#pragma import_defines ( ENABLE_DEPTH_OFFSET )

#include "DepthPrecision.h"
#include "Ocean.h"
#include "Brdfs/BlinnPhong.h"
#include "Brdfs/Lambert.h"
#include "ThirdParty/BruentonOcean.h"

in vec3 positionRelCameraWS;
in vec3 positionWS;
in float logZ;
in vec2 wrappedNoiseCoord;
in vec3 sunIrradiance;
in vec3 skyIrradiance;
in vec3 transmittance;
in vec3 skyRadianceToPoint;

out vec4 color;

uniform sampler2D normalSamplers[NUM_OCEAN_CASCADES];
uniform sampler2D foamMaskSamplers[NUM_OCEAN_CASCADES];
uniform sampler2D foamSampler;

uniform mat4 reflectionViewProjectionMatrix;
uniform vec2 heightMapTexCoordScales[NUM_OCEAN_CASCADES];
uniform vec3 lightDirection; // direction to light
uniform float waterHeight;
uniform vec3 ambientLightColor;

#ifdef ENABLE_DEPTH_OFFSET
	uniform float depthOffset;
#endif

uniform samplerBuffer wakeHashMapTexture;
uniform samplerBuffer wakeParamsTexture;

const float infinity = 1e10;
const float normalMipmapBias = 1.0; // TODO: why is this needed?
const float drawDistance = 80000;


float nearestPointOnLine(vec2 lineP0, vec2 lineP1, vec2 point)
{
	vec2 v0 = point - lineP0;
	vec2 v1 = lineP1 - lineP0;

	float v1_v1 = dot(v1, v1);
	if (v1_v1 == 0.0f) // if line has 0 length
		return 0.f;

	float v0_v1 = dot(v0, v1);

	float t = v0_v1 / v1_v1;

	if (t < 0.0f)
		t = 0.0f;
	else if (t > 1.0f)
		t = 1.0f;

	return t;
}

vec3 frontFacing(vec3 viewDirection, vec3 v)
{
	v *= sign(dot(viewDirection, v));
	return v;
}

// Manual texture lookup with bilinear filtering.
// Used in cases where more precision is required than used in built-in bilinear filtering.
vec4 bilinear(sampler2D tex, vec2 uv)
{
	vec2 dims = vec2(1201, 1201); // TODO: get actual dimensions
	vec2 weight = fract(uv * dims);
	vec2 texelSize = 1.0f / dims;
	
	uv -= weight * texelSize;
	
	vec4 c00 = textureGrad(tex, uv, vec2(0), vec2(0)); // TODO: would be good if we didn't have to use zero derivatives
	vec4 c10 = textureGrad(tex, uv + vec2(texelSize.x, 0), vec2(0), vec2(0));
	vec4 c01 = textureGrad(tex, uv + vec2(0, texelSize.y), vec2(0), vec2(0));
	vec4 c11 = textureGrad(tex, uv + texelSize, vec2(0), vec2(0));
	
	vec4 c0 = mix(c00, c10, weight.x);
	vec4 c1 = mix(c01, c11, weight.x);
	return mix(c0, c1, weight.y);
}

vec3 blendNormals(vec3 baseColor, vec3 detailColor)
{
	// Reoriented Normal Blending
	// https://blog.selfshadow.com/publications/blending-in-detail/
	vec3 t = baseColor * vec3( 2,  2, 2) + vec3(-1, -1,  0);
	vec3 u = detailColor * vec3(-2, -2, 2) + vec3( 1,  1, -1);
	return normalize(t*dot(t, u) - u*t.z);
}

vec3 calculateReflectionDirection(vec3 viewDirection, vec3 normal)
{
	vec3 reflected = reflect(-viewDirection, normal);
	
	// Adjust reflected ray to not reflect too low on the reflection map (otherwise we will reflect area 'underneath' objects near interfaces)
	reflected.z = min(reflected.z, 0); // don't reflect under ocean

// Reflections at a grazing angle can reflect a point 'below' the water-terrain interface
// because we assume all reflection sources are at infinity.
// To compensate for this, here we smooth the water at grazing angles
//#define GRAZING_REFLECTION_FIX
#ifdef GRAZING_REFLECTION_FIX
	vec3 reflectedFromUp = reflect(-viewDirection, vec3(0,0,-1));
	reflected = mix(reflectedFromUp, reflected, clamp(-viewDirection.z*0.7+0.5, 0.0, 1.0));
	reflected = normalize(reflected);
#endif
	return reflected;
}

vec2 calculateReflectionTexCoord(vec3 viewDirection, vec3 normal)
{
	vec3 reflected = calculateReflectionDirection(viewDirection, normal);

	vec4 reflectionSourcePosition = vec4(positionWS + reflected * infinity, 1);
	vec4 screenCoord = reflectionViewProjectionMatrix * reflectionSourcePosition;
	screenCoord.xyz /= screenCoord.w;
	return screenCoord.xy * 0.5 + 0.5;
}

float distanceSquared(vec2 a, vec2 b)
{
	vec2 c = a-b;
	return dot(c,c);
}

float calcSternWakeMask(vec3 wakeStart, vec3 wakeEnd)
{
	vec2 positionWS2d = positionWS.xy;
	float t = nearestPointOnLine(wakeStart.xy, wakeEnd.xy, positionWS2d);
	
	vec3 wakeStartToEnd = wakeEnd - wakeStart;
	vec3 pointOnWake = wakeStart + wakeStartToEnd * t;

	vec2 dir = normalize(wakeStartToEnd.xy);
	vec2 lateral = vec2(dir.y, -dir.x);
	pointOnWake.xy += lateral * sin(t*100)*0.5;
	
	float s = distance(pointOnWake.xy, positionWS2d);
	
	float sternWake = clamp(0.5 * (pointOnWake.z - s), 0.0, 1.0) * (1.0 - t);
	
	// Add bands of lesser amount of foam
	sternWake *= 0.3 + 0.7 * clamp(0.2 * (abs(s-pointOnWake.z*0.4))-0.1, 0.0, 1.0);
	
	return sternWake;
}

float calcRotorWashMask(vec3 wakeStart, vec3 wakeEnd)
{
	vec2 wakeDir = normalize(wakeEnd.xy - wakeStart.xy);
	wakeStart.xy += wakeDir * 10;
	vec2 positionWS2d = positionWS.xy;
	float elongate = dot(wakeDir, normalize(positionWS2d.xy - wakeStart.xy)) * 0.5 + 0.5;
	float radius = length(wakeStart.xy - positionWS2d);
	float foamRadius = mix(15, 25, elongate);
	float doughnut = 0.12 * (1.0 - 0.5 * elongate) * max(0.0, 1.0 - 0.05 * abs(radius - foamRadius)) * mix(0.2, 1.0, 0.5 * (1.0 + sin(radius)));
	return doughnut;
}

const int hashCellsX = 128;
vec2 cellWorldSize = vec2(100);

int modPositive(int a, int b)
{
	return (a%b+b)%b;
}

int calcHash(vec2 position)
{
	vec2 cellF = position / cellWorldSize;
	ivec2 cell = ivec2(floor(cellF));
	return modPositive(cell.x, hashCellsX) + modPositive(cell.y, hashCellsX) * hashCellsX;
}

float calcWakeMask()
{
	int hashIndex = calcHash(positionWS.xy) * 2;
	int paramsBeginIndex = int(texelFetch(wakeHashMapTexture, hashIndex).x);
	int paramsEndIndex = int(texelFetch(wakeHashMapTexture, hashIndex + 1).x); // last index + 1

	float mask = 0;
	for (int i = paramsBeginIndex; i < paramsEndIndex; i += 2)
	{
		vec3 wakeStart = texelFetch(wakeParamsTexture, i).xyz;
		vec3 wakeEnd = texelFetch(wakeParamsTexture, i+1).xyz;
		if (wakeStart.z < 0.0)
		{
			mask = max(mask, calcRotorWashMask(wakeStart, wakeEnd));
		}
		else
		{
			mask = max(mask, calcSternWakeMask(wakeStart, wakeEnd));
		}
	}

	return mask;
}

float calcUvCyclesPerPixel(vec2 uv)
{
	vec2 dxUv = dFdx(uv);
	vec2 dyUv = dFdy(uv);
	return max(dot(dxUv, dxUv), dot(dyUv, dyUv));
}

const vec3 upDir = vec3(0,0,-1);

void main(void)
{
	float viewDistance = length(positionRelCameraWS);
#ifdef DISTANCE_CULL
	if (viewDistance > drawDistance)
	{
		discard;
		return;
	}
#endif
	vec2 texCoord = wrappedNoiseCoord * heightMapTexCoordScales[0];
	vec3 normalSample = texture(normalSamplers[0], texCoord, normalMipmapBias).rgb;
	float foamMask = texture(foamMaskSamplers[0], texCoord).r;

	vec3 normal = normalSample * vec3(2) - vec3(1);
	float normalLength = length(normal);
	
	if (NUM_OCEAN_CASCADES == 1)
	{
		normal = normal / normalLength;
	}
	else
	{
		for (int i = 1; i < NUM_OCEAN_CASCADES; ++i)
		{
			vec2 texCoord = wrappedNoiseCoord * heightMapTexCoordScales[i];
			
			vec3 normalSample2 = texture(normalSamplers[i], texCoord, normalMipmapBias).rgb;
			normal = blendNormals(normalSample, normalSample2);
			foamMask = max(foamMask, texture(foamMaskSamplers[i], texCoord).r);
		}
	}

	normal.z = -normal.z;

	vec3 viewDirection = -positionRelCameraWS / viewDistance;

    if (dot(viewDirection, normal) < 0.0) {
        normal = reflect(normal, viewDirection); // reflects backfacing normals
    }
	
	
	vec3 u = positionRelCameraWS; // MTODO
	vec3 N = -normal;
	vec3 V = -viewDirection; // MTODO: unflip. +z up?
	vec3 L = -lightDirection;
	
	
    float Jxx = dFdx(u.x);
    float Jxy = dFdy(u.x);
    float Jyx = dFdx(u.y);
    float Jyy = dFdy(u.y);
    float A = Jxx * Jxx + Jyx * Jyx;
    float B = Jxx * Jxy + Jyx * Jyy;
    float C = Jxy * Jxy + Jyy * Jyy;
    const float SCALE = 10.0;
    float ua = pow(A / SCALE, 0.25);
    float ub = 0.5 + 0.5 * B / sqrt(A * C);
    float uc = pow(C / SCALE, 0.25);
    vec2 sigmaSq = vec2(0.0001);// MTODO texture3D(slopeVarianceSampler, vec3(ua, ub, uc)).xw;

    sigmaSq = max(sigmaSq, 2e-5);

    vec3 Ty = normalize(vec3(0.0, N.z, -N.y));
    vec3 Tx = cross(Ty, N);

    float fresnel = 0.02 + 0.98 * meanFresnel(V, N, sigmaSq);

	color.rgb = vec3(0);
    color.rgb += reflectedSunRadiance(L, V, N, Tx, Ty, sigmaSq) * sunIrradiance;

    color.rgb += fresnel * meanSkyRadiance(V, N, Tx, Ty, sigmaSq);

    vec3 Lsea = deepScatterColor * skyIrradiance / M_PI;
    color.rgb += (1.0 - fresnel) * Lsea;
	
	color.rgb += ambientLightColor;
	
	float foamEnergyNormalizationTerm = 1.5 * M_PI; // This is a guess. Presumably the real value is somewhere between pi and 2 * pi.
	vec3 foamReflectance = (sunIrradiance + skyIrradiance) / foamEnergyNormalizationTerm;

	foamMask = max(foamMask, calcWakeMask());

	float fadeFactor = viewDistance / drawDistance;

	vec3 foamColor = vec3(1.0);
	float foamTexture = texture(foamSampler, wrappedNoiseCoord.xy*400).r;
	float foamFraction = pow(foamTexture, 0.5 / (foamMask + 0.0001)) * smoothstep(0.0, 0.1, foamMask);
	
	color.rgb = mix(color.rgb, foamColor * foamReflectance, foamFraction);
	
	color.rgb = color.rgb * transmittance + skyRadianceToPoint;

#ifdef DISTANCE_CULL
	color.a = min(1.0, 1.5 - 1.5 * fadeFactor);
#else
	color.a = 1.0;
#endif
	gl_FragDepth = logarithmicZ_fragmentShader(logZ);
#ifdef ENABLE_DEPTH_OFFSET
	gl_FragDepth += depthOffset;
#endif
}