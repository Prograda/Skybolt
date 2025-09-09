/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#version 440 core
#pragma import_defines ( DISTANCE_CULL )
#pragma import_defines ( ENABLE_DEPTH_OFFSET )

#extension GL_EXT_gpu_shader4 : enable // Required by HARDWARE_ANISTROPIC_FILTERING
#define HARDWARE_ANISTROPIC_FILTERING

#include "AtmosphericScatteringWithClouds.h"
#include "DepthPrecision.h"
#include "Ocean.h"
#include "Rerange.h"
#include "Wake.h"
#include "Brdfs/BlinnPhong.h"
#include "Brdfs/Lambert.h"
#include "ThirdParty/BruentonOcean.h"

in vec3 positionRelCameraWS;
in vec3 positionWS;
in float logZ;
in vec2 wrappedNoiseCoord;
in AtmosphericScattering scattering;

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

const float infinity = 1e10;
const float normalMipmapBias = 1.0; // TODO: why is this needed?

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

float calcUvCyclesPerPixel(vec2 uv)
{
	vec2 dxUv = dFdx(uv);
	vec2 dyUv = dFdy(uv);
	return max(dot(dxUv, dxUv), dot(dyUv, dyUv));
}

vec3 calcSphereLightDir(vec3 R, vec3 L)
{
	// Based on https://alextardif.com/arealights.html
	vec3 centerToRay = (dot(L, R) * R) - L;

	vec3 closestPoint = L + centerToRay * min(1.0, 0.005 / length(centerToRay));
	return normalize(closestPoint);
}

const vec3 upDir = vec3(0,0,-1);

void main(void)
{
	float viewDistance = length(positionRelCameraWS);
#ifdef DISTANCE_CULL
	if (viewDistance > oceanMeshFadeoutEndDistance)
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
			foamMask = max(foamMask, texture(foamMaskSamplers[i], texCoord, normalMipmapBias).r);
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
	vec3 L = -calcSphereLightDir(reflect(V, N), lightDirection);
	
	
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
    vec2 sigmaSq = vec2(0.001);// TODO texture3D(slopeVarianceSampler, vec3(ua, ub, uc)).xw;

    sigmaSq = max(sigmaSq, 2e-5);

    vec3 Ty = normalize(vec3(0.0, N.z, -N.y));
    vec3 Tx = cross(Ty, N);

    float fresnel = 0.02 + 0.98 * meanFresnel(V, N, sigmaSq);

	color.rgb = vec3(0);
    color.rgb += reflectedSunRadiance(L, V, N, Tx, Ty, sigmaSq) * scattering.sunIrradiance;

    color.rgb += fresnel * meanSkyRadiance(V, N, Tx, Ty, sigmaSq);

    vec3 Lsea = deepScatterColor * scattering.skyIrradiance / M_PI;
    color.rgb += (1.0 - fresnel) * Lsea;
	
	color.rgb += ambientLightColor;
	
	float foamEnergyNormalizationTerm = 1.5 * M_PI; // This is a guess. Presumably the real value is somewhere between pi and 2 * pi.
	vec3 foamReflectance = (scattering.sunIrradiance + scattering.skyIrradiance) / foamEnergyNormalizationTerm;

	float bowWakeStrength = 0.5;
	float sternWakeStrength = 0.9;
	foamMask = max(foamMask, calcWakeMask(positionWS.xy, bowWakeStrength, sternWakeStrength).x);

	vec3 foamColor = vec3(1.0);
	float foamTexture = texture(foamSampler, wrappedNoiseCoord.xy*400, normalMipmapBias).r;
	float foamFraction = pow(foamTexture, 2 / (foamMask + 0.0001)) * smoothstep(0.0, 0.1, foamMask);
	foamFraction += foamTexture * foamMask * 0.15; // simulate bubbles under surface
	foamFraction = clamp(foamFraction, 0, 1);
	
	color.rgb = mix(color.rgb, foamColor * foamReflectance, foamFraction);
	
	color.rgb = color.rgb * scattering.transmittance + scattering.skyRadianceToPoint;

#ifdef DISTANCE_CULL
	color.a = min(1.0, (oceanMeshFadeoutEndDistance - viewDistance) / (oceanMeshFadeoutEndDistance - oceanMeshFadeoutStartDistance));
#else
	color.a = 1.0;
#endif
	gl_FragDepth = logarithmicZ_fragmentShader(logZ);
#ifdef ENABLE_DEPTH_OFFSET
	gl_FragDepth += depthOffset;
#endif
}