/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#version 440 core

#include "AtmosphericScatteringWithClouds.h"
#include "Clouds.h"
#include "DepthPrecision.h"
#include "GlobalDefines.h"
#include "Planet.h"
#include "RaySphereIntersection.h"
#include "Brdfs/HenyeyGreenstein.h"
#include "Util/Checker.h"

in vec3 vertCameraWorldDir;
in float cameraAltitude;
in vec2 screenCoord;

layout(location = 0) out vec4 colorOut;
layout(location = 1) out float depthOut;

uniform sampler2D globalAlphaSampler;
uniform sampler2D coverageDetailSampler;
uniform sampler3D noiseVolumeSampler;
uniform sampler2D sceneDepthSampler;

uniform vec3 cameraPosition;
uniform vec3 lightDirection;
uniform vec3 cameraCenterDirection;
uniform vec3 ambientLightColor;
uniform float upscaleTextureLodFactor;
uniform vec2 jitterOffset;

// Cloud geometry heights for each cloud type
const float cloudLayerMinHeight = 1000;
const float cloudLayerMaxHeight = 7000;
const vec4 cloudBottomZeroDensity = vec4(1000, 4100, 6700, 0);
const vec4 cloudTopZeroDensityHeight = vec4(2000, 5000, 7000, 0);
const vec4 cloudDensityMultiplier = vec4(0.03, 0.02, 0.0002, 0);
const vec4 cloudOcclusionStrength = vec4(0.1, 0.1, 0, 0);
const vec4 cloudNoiseStrength = vec4(0.2, 0.2, 0.1, 0);

const vec3 noiseFrequencyScale = vec3(0.0003);

// Returns an apprxomate semicircle over x in range [0, 1]
vec4 semicircle(vec4 x)
{
	vec4 a = clamp(x * vec4(2) - vec4(1), -vec4(1), vec4(1));
	return vec4(1) - a*a;
}

vec4 biasedSemiCircle(vec4 x, vec4 bias)
{
	return semicircle(pow(x, bias));
}

vec4 calcHeightMultiplier(float height)
{
	vec4 h = remapNormalized(vec4(height), cloudBottomZeroDensity, cloudTopZeroDensityHeight);
	return biasedSemiCircle(h, vec4(0.5));
}

const int iterations = 1000;
const float initialStepSize = 100;
const float maximumStepSize = 500;
const float stepSizeGrowthFactor = 1.002;
const float maxRenderDistance = 300000;

DensityHullSample sampleDensityHull(vec2 uv, float height, float lod)
{
	DensityHullSample s;
	s.coverageBase = sampleBaseCloudCoverage(globalAlphaSampler, uv);
	s.coverageDetail = sampleCoverageDetail(coverageDetailSampler, uv, lod);
	s.heightMultiplier = calcHeightMultiplier(height);
	return s;
}

//! @param lod is 0 for maximum detail. Detail falls off with log2 (as per mipmap lod level)
float calcDensity(DensityHullSample s, vec3 pos, float lod)
{
	vec4 density = calcCloudDensityLowRes(s);
	
	float lowDetailNoise = 1 - textureLod(noiseVolumeSampler, pos*noiseFrequencyScale*0.57, 0).r;
	density  *= min(vec4(1), pow(mix(vec4(1), 1.5*vec4(lowDetailNoise), cloudNoiseStrength), vec4(15)));
	
	// Apply detail
	float detailBlend = lod*0.3;
	if (detailBlend < 1)
	{
		float noise = textureLod(noiseVolumeSampler, pos*noiseFrequencyScale*1.3, 0).r;
		
		float filteredNoise = pow((1.0-noise), 6)*0.4;
		vec4 highResDensity = clamp(remapNormalized(density, vec4(filteredNoise), vec4(1)), vec4(0), vec4(1));
		density = mix(density, highResDensity, vec4(1) - vec4(detailBlend));
	}
	
	return dot(density, cloudDensityMultiplier);
}

const vec3 RANDOM_VECTORS[6] = vec3[6]
(
	vec3( 0.38051305f,  0.92453449f, -0.02111345f),
	vec3(-0.50625799f, -0.03590792f, -0.86163418f),
	vec3(-0.32509218f, -0.94557439f,  0.01428793f),
	vec3( 0.09026238f, -0.27376545f,  0.95755165f),
	vec3( 0.28128598f,  0.42443639f, -0.86065785f),
	vec3(-0.16852403f,  0.14748697f,  0.97460106f)
);

const float SAMPLE_DISTANCES[6] = float[6](1, 2, 4, 8, 16, 32);
const float SAMPLE_SEGMENT_LENGTHS[6] = float[6](1, 1, 2, 4, 8, 16);

const int lightSampleCount = 6;
const vec3 albedo = vec3(0.95);

const float powderScale = 300.0;
const float powderExponent = 0.2;
const float scatterDistanceMultiplier = 0.5; // less than 1 to fake multi-scattering
const float scatterSampleDistanceScale = 200;

vec3 radianceLowRes(vec3 pos, vec2 uv, vec3 lightDir, float hg, vec3 sunIrradiance, float lod, float height, float density)
{
	float Texponent = 0.0;
	float powderTexponent = 0.0;
	
	for (int i = 0; i < lightSampleCount; ++i)
	{
		float scatterDistance = SAMPLE_SEGMENT_LENGTHS[i] * scatterSampleDistanceScale;
	
		vec3 lightSamplePos = pos + (lightDir + RANDOM_VECTORS[i] * 0.3) * SAMPLE_DISTANCES[i] * scatterSampleDistanceScale;
		float sampleHeight = length(lightSamplePos) - innerRadius;
		vec2 sampleUv = cloudUvFromPosRelPlanet(lightSamplePos);
		float density = calcDensity(sampleDensityHull(sampleUv, sampleHeight, lod), lightSamplePos, lod);
		Texponent -= density * scatterDistance * scatterDistanceMultiplier;
	}

	float T = exp(Texponent);
	
	vec4 occlusionByType = clamp(vec4(1) - remapNormalized(vec4(height), cloudBottomZeroDensity, cloudTopZeroDensityHeight), vec4(0), vec4(1));
	occlusionByType *= cloudOcclusionStrength;
	float occlusion = max(0, 1 - dot(occlusionByType, vec4(1.0)));
	vec3 K = occlusion * sunIrradiance * albedo;

	vec3 radiance = vec3(0);
	float albedoMScat = 1;
	for (int N = 0; N < 2; ++N) // multi scattering octaves
	{
		// Calculate multi-scattering approximation coefficeints for technique described in
		// "Oz: The Great and Volumetric"
		// http://magnuswrenninge.com/wp-content/uploads/2010/03/Wrenninge-OzTheGreatAndVolumetric.pdf
		// and also used in "Physically Based Sky, Atmosphere and Cloud Rendering in Frostbite"
		// https://media.contentapi.ea.com/content/dam/eacom/frostbite/files/s2016-pbs-frostbite-sky-clouds-new.pdf
	
		float extinctionMScat = albedoMScat;
		float hgMScat = albedoMScat;
	
		// Now perform single scattering calc modified by the coefficients for fake multiscattering
		float beer = pow(T, extinctionMScat);
		float powder = min(1.0, pow(density * powderScale, powderExponent));
		
		radiance += K * albedoMScat * beer * powder * mix(oneOnFourPi, hg, hgMScat);
		albedoMScat *= 0.5;
	}
	// Fudge factors to account for missing energy from not considering higher multiscattering octaves (N=inf)
	const float multiScatterTailA = 1.1;
	const vec3 multiScatterTailB = occlusion * sunIrradiance * 0.01;

	return multiScatterTailA * radiance + multiScatterTailB + ambientLightColor;
}

float texelsPerPixelAtDistance(float distance)
{
	return distance/120000.0;
}

float effectiveZeroT = 0.01;
float effectiveZeroDensity = 0.00001;

vec4 march(vec3 start, vec3 dir, float stepSize, float tMax, float rayStartTexelsPerPixel, vec3 lightDir, vec3 sunIrradiance, out float meanCloudDistance)
{
	float cosAngle = dot(lightDir, dir);
	float hg = watooHenyeyGreenstein(cosAngle);
	
    // The color that is accumulated during raymarching
    vec3 totalRadiance = vec3(0, 0, 0);
    float T = 1.0;

	// As we march, also calculate the transmittance-weigthed mean cloud scene depth.
	// Based on "Physically Based Sky, Atmosphere and Cloud Rendering in Frostbite"
	// section 5.9.1 Aerial perspective affecting clouds
	// https://media.contentapi.ea.com/content/dam/eacom/frostbite/files/s2016-pbs-frostbite-sky-clouds-new.pdf
	float sumTransmissionWeightedDistance = 0.0;
	float sumTransmissionWeights = 0.0;
	
    // Do the raymarching
    float t = 0.0;
	bool lastStep = false;
    for (int i = 0; i < iterations; ++i)
	{
		vec3 pos = start + dir * t;
		float lod = log2(max(1.0, rayStartTexelsPerPixel + texelsPerPixelAtDistance(t)));
		
		vec2 uv = cloudUvFromPosRelPlanet(pos);
		float height = length(pos) - innerRadius;
			
		// Sample density hull
		DensityHullSample densityHullSample = sampleDensityHull(uv, height, lod);
		vec4 d = calcCloudDensityHull(densityHullSample);
			
		// If empty space, take a big step
		if (dot(d, vec4(1)) <= effectiveZeroDensity)
		{
			float bigStepSize = mix(stepSize, maximumStepSize, min(1.0, lod));
			t += bigStepSize;
		}
		else
		{
			// Integrate
			float density = calcDensity(densityHullSample, pos + vec3(cloudDisplacementMeters.xy,0), lod);
			
			if (density > effectiveZeroDensity)
			{
				vec3 radiance = radianceLowRes(pos, uv, lightDir, hg, sunIrradiance, lod, height, density) * density;
	#define ENERGY_CONSERVING_INTEGRATION
	#ifdef ENERGY_CONSERVING_INTEGRATION
				const float clampedDensity = max(density, 0.0000001);

				// Energy conserving intergation from https://media.contentapi.ea.com/content/dam/eacom/frostbite/files/s2016-pbs-frostbite-sky-clouds-new.pdf
				float transmittance = exp(-density * stepSize);

				vec3 integScatt = (radiance - radiance * transmittance) / clampedDensity;

				totalRadiance += T * integScatt;
				T *= transmittance;
	#else
				totalRadiance += radiance * stepSize * T;
				T *= exp(-density * stepSize);
	#endif
				float transmissionWeight = transmittance * T;
				sumTransmissionWeightedDistance += transmissionWeight * t;
				sumTransmissionWeights += transmissionWeight;
			}
			
			// Take a small step
			stepSize *= stepSizeGrowthFactor;
			t += stepSize;
		}

        // early ray termination
        if(T <= effectiveZeroT) break;

		if (lastStep)
		{
			break;
		}
		
		if (t > tMax)
		{
			t = tMax;
			lastStep = true;
		}
    }

	meanCloudDistance = (sumTransmissionWeights > 0) ? (sumTransmissionWeightedDistance / sumTransmissionWeights) : -1.0;
	
    return vec4(totalRadiance, 1.0-max(0.0, remapNormalized(T, effectiveZeroT, 1.0)));
}

// Remaps value from 0 to 1 across pixelCount pixels.
float antiAliasSmoothStep(float value, float pixelCount)
{
	float width = max(abs(dFdx(value)), abs(dFdy(value))) * pixelCount;
	return smoothstep(0, width, value);
}

const float lowResBrightnessMultiplier = 1.3; // fudge factor to make low res clouds match brightness of high res clouds, needed because low res does not simulate any scattering

// Simulates colour change due to scattering
vec3 desaturate(vec3 c)
{
	return vec3(length(c));
}

vec4 evaluateGlobalLowResColor(vec2 cloudsUv, vec3 irradiance, float rayFar)
{
	// Calculate texture LOD level.
	// There is a singularity when the U coordinate wraps from 1 to 0, which we avoid by querying LOD
	// at U and fract(U-epsilon) and taking the minimum. We also tried Tarini's method (https://www.shadertoy.com/view/7sSGWG) which alleviated the seam but did not completely remove it.
	vec2 lod2d = textureQueryLod(globalAlphaSampler, cloudsUv.xy);
	float lod = max(lod2d.x, lod2d.y);
	lod2d = textureQueryLod(globalAlphaSampler, vec2(fract(cloudsUv.x-0.01), cloudsUv.y));
	lod = min(lod, max(lod2d.x, lod2d.y))-log2(upscaleTextureLodFactor);

	float alpha = textureLod(globalAlphaSampler, cloudsUv, lod).r;
	vec3 color = irradiance * alpha * oneOnFourPi;
	color = mix(color, desaturate(color), 0.15); // desaturate color to simulate scattering. This makes sunsets less extreme.
	return vec4(color, alpha);
}

float mipMapLevel(sampler2D sampler, vec2 uv)
{
	vec2 texCoord = uv * textureSize(sampler, 0);
	vec2  dtc_dx  = dFdx(texCoord);
	vec2  dtc_dy  = dFdy(texCoord);

	// Take the average of the x and y dimensions.
	// This provides a balance between sharpness and aliasing at grazing angles.
	float deltaMaxSqr = 1.0/upscaleTextureLodFactor * 0.5 * (dot(dtc_dx, dtc_dx) + dot(dtc_dy, dtc_dy));
	return 0.5 * log2(max(1.0, deltaMaxSqr));
}

float random(vec2 texCoord)
{
	float dotProduct = dot(texCoord, vec2(12.9898, 78.233));
	return fract(sin(dotProduct) * 43758.5453);
}

void main()
{
	vec3 rayDir = normalize(vertCameraWorldDir);
	
	float rayNear;
	float rayFar;

	bool hitPlanet = (raySphereFirstIntersection(cameraPosition, rayDir, planetCenter, innerRadius) >= 0.0);

	float alpha = 1;
	
	if (cameraAltitude < cloudLayerMinHeight)
	{
		if (hitPlanet)
		{
			colorOut = vec4(0.0);
			depthOut = 1.0;
			return;
		}

		rayNear = raySphereSecondIntersection(cameraPosition, rayDir, planetCenter, innerRadius + cloudLayerMinHeight);
		rayFar = raySphereSecondIntersection(cameraPosition, rayDir, planetCenter, innerRadius + cloudLayerMaxHeight);
		rayFar = min(rayFar, maxRenderDistance);
	}
	else if (cameraAltitude < cloudLayerMaxHeight)
	{
		rayNear = 0.0;

		if (hitPlanet)
		{
			// Far is the intersection with the cloud base
			rayFar = raySphereFirstIntersection(cameraPosition, rayDir, planetCenter, innerRadius + cloudLayerMinHeight);
		}
		else
		{
			// Far is the intersection with the cloud top
			rayFar = raySphereSecondIntersection(cameraPosition, rayDir, planetCenter, innerRadius + cloudLayerMaxHeight);
		}
	}
	else
	{
		float t0;
		float t1;
		raySphereIntersections(cameraPosition, rayDir, planetCenter, innerRadius + cloudLayerMaxHeight, t0, t1);
		rayNear = t0;
		
		if (hitPlanet)
		{
			// Far is the intersection with the cloud base
			rayFar = raySphereFirstIntersection(cameraPosition, rayDir, planetCenter, innerRadius + cloudLayerMinHeight);
		}
		else
		{
			// Far is the intersection with the cloud top
			rayFar = t1;
		}

		// Fade out clouds towards horizon a few pixels to fake antialiasing on rim of planet
		vec3 midPoint = cameraPosition + rayDir * (t0 + t1) * 0.5f;
		float alt = distance(planetCenter, midPoint);
		
		float signal = (innerRadius + cloudLayerMaxHeight - alt); // signal is 0 at horizon and positive below horizon
		alpha = antiAliasSmoothStep(signal, 5);
	}
	
	if (rayNear < 0.0)
	{
			colorOut = vec4(0.0);
			depthOut = 1.0;
			return;
	}

	float rayFarSansSceneDepth = rayFar;
	if (cameraAltitude < cloudLayerMaxHeight * 40)
	{
		float logZNdc = textureLod(sceneDepthSampler, screenCoord + jitterOffset, 0).r;
		float sceneDepthAlongRay = calcClipSpaceWFromLogZNdc(logZNdc) / dot(rayDir, cameraCenterDirection);
		rayFar = min(rayFar, sceneDepthAlongRay);
	}

	float stepSize = initialStepSize;
	rayNear += stepSize * random(gl_FragCoord.xy + jitterOffset);
	rayNear = min(rayNear - 0.0001, rayFar);

	vec3 positionRelCameraWS = rayNear * rayDir;

	vec3 cameraPositionRelPlanet = cameraPosition - planetCenter;
	vec3 positionRelPlanet = positionRelCameraWS + cameraPositionRelPlanet;
	vec3 positionRelPlanetPlanetAxes = mat3(planetMatrixInv) * positionRelPlanet;
	vec3 rayDirPlanetAxes = mat3(planetMatrixInv) * rayDir;
	vec3 lightDirPlanetAxes = mat3(planetMatrixInv) * lightDirection;
	
	vec3 positionRelPlanetSafe = positionRelPlanet*1.0003;
	
	vec3 skyIrradiance;
	vec3 sunIrradiance = GetSunAndSkyIrradiance(positionRelPlanetSafe, lightDirection, skyIrradiance);
	
	// Add sky radiance to sun radiance, to simulate multi scattering
	vec3 directIrradiance = (sunIrradiance + skyIrradiance);
	
	vec2 cloudsUv = cloudUvFromPosRelPlanet(positionRelPlanetPlanetAxes);
	float lod = mipMapLevel(coverageDetailSampler, cloudsUv.xy * vec2(300.0, 150.0));
	lod = mix(0, lod, min(1.0, 0.2 * cameraAltitude / cloudLayerMaxHeight));
	
//#define DEBUG_VOLUME
#ifdef DEBUG_VOLUME
	colorOut.rgb = vec3(checker(cloudsUv*vec2(2,1), 1000));
	colorOut.a = 1;
	depthOut = calcLogZNdc(dot(rayDir * rayNear, cameraCenterDirection));
	return;
#endif

	float lowResBlend = (lod - 2.0)*0.25;

	float meanCloudDistance; // negative value means no samples
	if (lowResBlend < 1.0)
	{
		float rayStartTexelsPerPixel = pow(2, lod);
		colorOut = march(positionRelPlanetPlanetAxes, rayDirPlanetAxes, stepSize, rayFar-rayNear, rayStartTexelsPerPixel, lightDirPlanetAxes, directIrradiance, meanCloudDistance);
	}
	else
	{
		meanCloudDistance = -1;
	}
	
	bool hasSample = (meanCloudDistance >= 0.0);
	
	float logZ;
	if (hasSample)
	{	
		meanCloudDistance += rayNear;
		{
			// Apply 'aerial perspective'
			vec3 transmittance;
			vec3 cloudFrontPositionRelPlanet = cameraPositionRelPlanet + meanCloudDistance * rayDir;

			// Decrease inscattering as a function of sky occlusion due to clouds.
			// This curve is a fudge, but gives plausible looking results.
			float cloudSkyOcclusion = sampleCloudSkyOcclusionMaskAtCloudsUv(globalAlphaSampler, cloudsUv);
			float heightFraction = clamp((cameraAltitude - cloudLayerMinHeight) / (cloudLayerMaxHeight - cloudLayerMinHeight), 0, 1);
			cloudSkyOcclusion = mix(cloudSkyOcclusion, 1.0, heightFraction);
			vec3 skyRadianceToPoint = GetSkyRadianceToPointWithCloudOcclusion(cameraPositionRelPlanet, cloudFrontPositionRelPlanet, lightDirection, cloudSkyOcclusion, transmittance);
			
			// Blend between no aerial perspective for 0 density and full aerial for 1 density.
			// This curve is a fudge, but gives plausible looking results.
			float weight = min(1.0, colorOut.a * colorOut.a * colorOut.a);
			colorOut.rgb = mix(colorOut.rgb, colorOut.rgb * transmittance + skyRadianceToPoint, weight);
		}
		logZ = calcLogZNdc(dot(rayDir * meanCloudDistance, cameraCenterDirection));
	}
	else
	{
		logZ = calcLogZNdc(dot(rayDir * rayFarSansSceneDepth, cameraCenterDirection));
	}

	if (lowResBlend > 0)
	{
		vec4 lowResColor = evaluateGlobalLowResColor(cloudsUv, directIrradiance, rayFar) * lowResBrightnessMultiplier;
		colorOut = mix(colorOut, vec4(lowResColor), min(1.0, lowResBlend));
	}

	colorOut *= alpha;

	// Store the square root of color to minimize banding artifacts by giving better precision at low color values.
	// We must sqare the value read from the output texture before use.
	colorOut.rgba = sqrt(colorOut.rgba);

	depthOut = logZ;	
}