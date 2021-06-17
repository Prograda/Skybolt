/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#version 440 core

#include "AtmosphericScattering.h"
#include "Clouds.h"
#include "DepthPrecision.h"
#include "GlobalDefines.h"
#include "Planet.h"
#include "RaySphereIntersection.h"

in vec3 vertCameraWorldDir;
in float cameraAltitude;

layout(location = 0) out vec4 colorOut;
layout(location = 1) out float depthOut;

uniform sampler2D globalAlphaSampler;
uniform sampler2D coverageDetailSampler;
uniform sampler3D baseNoiseSampler;

uniform vec3 cameraPosition;
uniform vec3 lightDirection;
uniform vec3 cameraCenterDirection;
uniform vec3 ambientLightColor;

// Cloud geometry heights for each cloud type
const float cloudLayerMaxHeight = 5000;
const float cloudLayerMinHeight = 2000;
const vec2 cloudTopZeroDensityHeight = vec2(3500, 5000);
const vec2 cloudTopFullDensityHeight = vec2(3000, 3850);
const vec2 cloudBottomFullDensityHeight = vec2(2800, 2800);
const vec2 cloudBottomZeroDensity = vec2(2400, 2000);
const vec2 cloudOcclusionStrength = vec2(0.4, 0.8);
const vec2 cloudDensityMultiplier = vec2(0.005, 0.005);

const vec3 noiseFrequencyScale = vec3(0.0002);

//! @param cloudTypes goes from 0 (small cloud) to 1 (big cloud)
float calcHeightMultiplier(float height, float cloudType)
{
	vec2 m = clamp(remap(vec2(height), cloudBottomZeroDensity, cloudBottomFullDensityHeight, vec2(0.0), vec2(1.0)), vec2(0.0), vec2(1.0))
		   * clamp(remap(vec2(height), cloudTopFullDensityHeight, cloudTopZeroDensityHeight, vec2(1.0), vec2(0.0)), vec2(0.0), vec2(1.0));
		   
	return mix(m.x, m.y, cloudType);
}

const int iterations = 1000;
const float initialStepSize = 50;
const float maximumStepSize = 500;
const float stepSizeGrowthFactor = 1.002;
const float maxRenderDistance = 300000;

float calcDensityLowRes(vec2 uv, float height, out float cloudType, vec2 lod)
{
	float coverageDetail = sampleCoverageDetail(coverageDetailSampler, uv, lod.x, cloudType);
	float heightMultiplier = calcHeightMultiplier(height, cloudType);
	return calcCloudDensityLowRes(globalAlphaSampler, uv, heightMultiplier, coverageDetail);
}

const float cloudChaos = 0.9;
const float averageNoiseSamplerValue = cloudChaos * 0.7;
		
//! @param lod is 0 for zero detail, 1 where frequencies of pos*noiseFrequencyScale should be visible
float calcDensity(vec3 pos, vec2 uv, vec2 lod, float height, out float cloudType)
{
	float density = calcDensityLowRes(uv, height, cloudType, lod);

	// Apply detail
	if (lod.y > 0)
	{
		float clampedLod = min(lod.y, 1.0);
	
		// MTODO: 3D noise texture has visible tiling artifacts when viewed along an axis, e.g at the equator. Need to fix this.
		vec4 noise = textureLod(baseNoiseSampler, pos*noiseFrequencyScale, 0); // MTODO: tweak
		
		// Apply high detail
//#define ENABLE_HIGH_DETAIL_CLOUDS
#ifdef ENABLE_HIGH_DETAIL_CLOUDS
		if (lod.y > 4)
		{
			float highDetailStrength = min(lod.y - 4, 0.7);
			noise.r = mix(noise.r, noise.r * textureLod(baseNoiseSampler, pos * noiseFrequencyScale * 5, 0).g, highDetailStrength);
		}
#endif		
		float filteredNoise = mix(averageNoiseSamplerValue, cloudChaos*noise.r, clampedLod); // filter cloud noise based on lod.
		
		density = clamp(remap(density, filteredNoise, 1.0, 0.0, 1.0), 0.0, 1.0);		
	}
	else
	{
		density = clamp(remap(density, averageNoiseSamplerValue, 1.0, 0.0, 1.0), 0.0, 1.0); // should look the same as smallest mipmap of the baseNoiseSampler texture
	}
	
	return density * mix(cloudDensityMultiplier.x, cloudDensityMultiplier.y, cloudType);
}

const float oneOnFourPi = 0.0795774715459;

vec4 henyeyGreenstein(float cosAngle, vec4 eccentricity)
{
    vec4 eccentricity2 = eccentricity*eccentricity;
    return oneOnFourPi * ((vec4(1.0) - eccentricity2) / pow(vec4(1.0) + eccentricity2 - 2.0*eccentricity*cosAngle, vec4(3.0/2.0)));
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
const vec3 albedo = vec3(1.0);

const float powderStrength = 0.2; // TODO: get this looking right
const float scatterDistanceMultiplier = initialStepSize * 1;

vec3 radianceLowRes(vec3 pos, vec2 uv, vec3 lightDir, float hg, vec3 sunIrradiance, vec2 lod, float height, float cloudType)
{
	// It looks better to make scatterDistance a fixed size, in this case the initialStepSize multiplied by a hand tuned factor.
	// Making it a function of the adpstepSize makes the lighting look inconsistent and less realistic.

	float Texponent = 0.0;
	float powerTexponent = 0.0;
	
	for (int i = 0; i < lightSampleCount; ++i)
	{
		float scatterDistance = SAMPLE_SEGMENT_LENGTHS[i] * scatterDistanceMultiplier;
	
		vec3 lightSamplePos = pos + (lightDir + RANDOM_VECTORS[i] * 0.8) * SAMPLE_DISTANCES[i] * scatterDistanceMultiplier;
		float sampleHeight = length(lightSamplePos) - innerRadius;
		vec2 sampleUv = cloudUvFromPosRelPlanet(lightSamplePos);
		float sampleOutCloudType;
		float density = calcDensity(lightSamplePos, sampleUv, lod, sampleHeight, sampleOutCloudType);
		Texponent -= density * scatterDistance;
		powerTexponent -= density * scatterDistance * 2;
	}

	float T = exp(Texponent);
	float powderT = exp(powerTexponent);
	
	vec2 occlusionByType = clamp(remap(vec2(height), cloudBottomZeroDensity, cloudTopZeroDensityHeight, vec2(0.0), vec2(1.0)), vec2(0.0), vec2(1.0));
	occlusionByType = occlusionByType * cloudOcclusionStrength + (1.0 - cloudOcclusionStrength);
	float occlusion = mix(occlusionByType.x, occlusionByType.y, cloudType);

	vec3 radiance = vec3(0);
	for (int N = 0; N < 3; ++N) // multi scattering octaves
	{
		// Calculate multi-scattering approximation coefficeints for technique described in
		// "Oz: The Great and Volumetric"
		// http://magnuswrenninge.com/wp-content/uploads/2010/03/Wrenninge-OzTheGreatAndVolumetric.pdf
		// and also used in "Physically Based Sky, Atmosphere and Cloud Rendering in Frostbite"
		// https://media.contentapi.ea.com/content/dam/eacom/frostbite/files/s2016-pbs-frostbite-sky-clouds-new.pdf
	
		float albedoMScat = pow(0.5, N);
		float extinctionMScat = albedoMScat;
		float hgMScat = albedoMScat;
	
		// Now perform single scattering calc modified by the coefficients for fake multiscattering
		float beer = pow(T, extinctionMScat);
		float powder = 1.0 - powderStrength * pow(powderT, extinctionMScat);
		
		radiance += occlusion * sunIrradiance * albedo * albedoMScat * beer * powder * mix(oneOnFourPi, hg, hgMScat);
	}
	radiance += ambientLightColor;
	return radiance;
}

float effectiveZeroT = 0.01;

vec4 march(vec3 start, vec3 dir, float stepSize, float tMax, vec2 lod, vec3 lightDir, vec3 sunIrradiance, out float meanCloudFrontDistance)
{
	float cosAngle = dot(lightDir, dir);

	// Tuned to match The "Watoo" phase function for clouds, from Bouthors et al.
	// See http://wiki.nuaj.net/index.php?title=Clouds
	float hg = dot(henyeyGreenstein(cosAngle, vec4(-0.2, 0.3, 0.96, 0)), vec4(0.5, 0.5, 0.03, 0.0));
	
    // The color that is accumulated during raymarching
    vec3 totalRadiance = vec3(0, 0, 0);
    float T = 1.0;

	// As we march, also calculate the transmittance-weigthed mean cloud scene depth.
	// Based on "Physically Based Sky, Atmosphere and Cloud Rendering in Frostbite"
	// section 5.9.1 Aerial perspective affecting clouds
	// https://media.contentapi.ea.com/content/dam/eacom/frostbite/files/s2016-pbs-frostbite-sky-clouds-new.pdf
	float sumTransmissionWeightedDistance = 0.0;
	float sumTransmissionWeights = 0.0;
	
    // Do the actual raymarching
    float t = 0.0;
	bool lastStep = false;
    for (int i = 0; i < iterations; ++i)
	{	
		vec3 pos = start + dir * t;
		
		vec2 uv = cloudUvFromPosRelPlanet(pos);
		float height = length(pos) - innerRadius;
		float outCloudType;
        float density = calcDensity(pos + vec3(cloudDisplacementMeters.xy,0), uv, lod, height, outCloudType);
		
		if (density > 0.0001)
		{
			vec3 radiance = radianceLowRes(pos, uv, lightDir, hg, sunIrradiance, lod, height, outCloudType) * density;
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
			sumTransmissionWeightedDistance += T * t;
			sumTransmissionWeights += T;
		}
		else
		{
			// Skip empty space
			for(;i < iterations; ++i)
			{
				t += maximumStepSize;
				if (t > tMax)
				{
					break;
				}
				
				vec3 pos = start + dir * t;
				
				uv = cloudUvFromPosRelPlanet(pos);
				float height = length(pos) - innerRadius;
				float outCloudType;
				float density = calcDensityLowRes(uv, height, outCloudType, lod);
				
				// If no longer in empty space, go back one step and break
				if (density > 0)
				{
					t -= maximumStepSize;
					break;
				}
			}
		}

        // early ray termination
        if(T <= effectiveZeroT) break;

		if (lastStep)
		{
			break;
		}
		
		stepSize *= stepSizeGrowthFactor;
		t += stepSize;
		if (t > tMax)
		{
			t = tMax;
			lastStep = true;
		}
    }

	meanCloudFrontDistance = (sumTransmissionWeights > 0) ? (sumTransmissionWeightedDistance / sumTransmissionWeights) : -1.0;
	
    return vec4(totalRadiance, 1.0-max(0.0, remapNormalized(T, effectiveZeroT, 1.0)));
}

// Remaps value from 0 to 1 across pixelCount pixels.
float antiAliasSmoothStep(float value, float pixelCount)
{
	float width = max(abs(dFdx(value)), abs(dFdy(value))) * pixelCount;
	return smoothstep(0, width, value);
}

const float lowResBrightnessMultiplier = 4.5; // fudge factor to make low res clouds match brightness of high res clouds, needed because low res does not simulate any scattering

// Simulates colour change due to scattering
vec3 desaturateAndBlueShift(vec3 c)
{
	return dot(c, vec3(0.1 / 3.0)) * vec3(0.6, 0.9, 1.4);
}

vec4 evaluateGlobalLowResColor(vec3 positionRelPlanetPlanetAxes, vec3 irradiance)
{
	float alpha = textureLod(globalAlphaSampler, cloudUvFromPosRelPlanet(positionRelPlanetPlanetAxes), 0).r; // MTODO: auto lod
	vec3 color = irradiance * alpha * oneOnFourPi;
	color = mix(color, desaturateAndBlueShift(color), 0.8); // desaturate color to simulate scattering. This makes sunsets less extreme.
	return vec4(color, alpha);
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
			colorOut.rgb = vec3(0.0);
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
			colorOut.rgb = vec3(0.0);
			depthOut = 1.0;
			return;
	}

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
	
	float stepSize = initialStepSize;
	stepSize = min(stepSize + rayNear / 10000, maximumStepSize);
	vec2 lod;
	lod.x = smoothstep(0.1, 1.0, stepSize / maximumStepSize);
	lod.y = max(0.0, 150000 / (rayNear + 0.1) - 1.0);
	
	
	float meanCloudFrontDistance; // negative value means no samples
	colorOut = march(positionRelPlanetPlanetAxes, rayDirPlanetAxes, stepSize, rayFar-rayNear, lod, lightDirPlanetAxes, directIrradiance, meanCloudFrontDistance);
	
	bool hasSample = (meanCloudFrontDistance >= 0.0);
	meanCloudFrontDistance += rayNear;
	
	float logZ;
	if (lod.x < 0.99 && hasSample)
	{
		logZ = calcLogZNdc(dot(rayDir * meanCloudFrontDistance, cameraCenterDirection));
	}
	else
	{
		float precisionBias = 0.99; // needed to avoid z fighting at extreme distances
		logZ = calcLogZNdc(precisionBias*dot(rayDir * rayFar, cameraCenterDirection));
	}

	float lowResColorMultiScatterMultiplier = 1.5;
	vec4 lowResColor = evaluateGlobalLowResColor(positionRelPlanetPlanetAxes, directIrradiance) * lowResBrightnessMultiplier;
	colorOut = mix(colorOut, vec4(lowResColor), lod.x);

	if (hasSample)
	{
		// Apply 'aerial perspective'
		vec3 transmittance;
		vec3 cloudFrontPositionRelPlanet = cameraPositionRelPlanet + meanCloudFrontDistance * rayDir;
		vec3 skyRadianceToPoint = GetSkyRadianceToPoint(cameraPositionRelPlanet, cloudFrontPositionRelPlanet, 0, lightDirection, transmittance);
		
		float weight = min(1.0, colorOut.a * colorOut.a * colorOut.a); // Blend between no aerial perspective for 0 density and full aerial for 1 density. This curve is a fudge, but looks about right.
		colorOut.rgb = mix(colorOut.rgb, colorOut.rgb * transmittance + skyRadianceToPoint, weight);
	}

	colorOut *= alpha;

	// Store the square root of color to minimize banding artifacts by giving better precision at low color values.
	// We must sqare the value read from the output texture before use.
	colorOut.rgba = sqrt(colorOut.rgba);
	
	depthOut = logZ;	
}