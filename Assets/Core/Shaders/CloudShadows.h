/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef CLOUD_SHADOWS_H
#define CLOUD_SHADOWS_H

#include "Clouds.h"
#include "GlobalUniforms.h"

const float cloudAltitude = 5000;

uniform sampler2D coverageDetailSampler2;

float calcDensityLowRes2(sampler2D cloudSampler, vec2 uv)
{
	float cloudType;
	float lod = 0.0;
	float coverageBase = sampleBaseCloudCoverage(cloudSampler, uv);
	float coverageDetail = sampleCoverageDetail(coverageDetailSampler2, uv, lod, cloudType);

	float density = calcCloudDensityLowRes(coverageBase, coverageDetail, /* heightMultiplier */ 1.0).r;
	return clamp(remap(density, 0.4, 0.6, 0.0, 1.0), 0.0, 1.0);
}

float sampleCloudShadowMask(sampler2D cloudSampler, vec2 cloudTexCoord, vec3 lightDirection)
{
	vec2 cloudProjectionWorldDistance = vec2(lightDirection.xy) * cloudAltitude / max(0.05, -lightDirection.z);
	vec2 uv = offsetCloudUvByWorldDistance(cloudTexCoord, cloudProjectionWorldDistance);
	float mask = 1.0 - calcDensityLowRes2(cloudSampler, uv);
	return mask;
}

float sampleCloudShadowMaskAtTerrainUv(sampler2D cloudSampler, vec2 geoTexCoord, vec3 lightDirection)
{
	vec2 cloudTexCoord = cloudUvFromTerrainUv(geoTexCoord.xy);
	return sampleCloudShadowMask(cloudSampler, cloudTexCoord, lightDirection);
}

float sampleCloudAlphaAtPositionRelPlanet(sampler2D cloudSampler, vec3 positionRelPlanet, vec3 rayDirection)
{
	vec3 positionRelPlanetPlanetAxes = mat3(planetMatrixInv) * positionRelPlanet;
	vec2 uv = cloudUvFromPosRelPlanet(positionRelPlanetPlanetAxes);
	vec2 cloudProjectionWorldDistance = vec2(rayDirection.xy) * cloudAltitude / -rayDirection.z;
	return 0.1 * calcDensityLowRes2(cloudSampler, offsetCloudUvByWorldDistance(uv, cloudProjectionWorldDistance));
}

float sampleCloudShadowMaskAtCloudsUv(sampler2D cloudSampler, vec2 uv, float altitude, vec3 lightDirection)
{
	float mask = sampleCloudShadowMask(cloudSampler, uv, lightDirection);
	return mix(mask, 1.0, clamp(remapNormalized(altitude, 2000, 5000), 0.0, 1.0)); // TODO: use actual cloud heights
}

float sampleCloudShadowMaskAtPositionRelPlanet(sampler2D cloudSampler, vec3 positionRelPlanet, vec3 lightDirection)
{
	vec3 positionRelPlanetPlanetAxes = mat3(planetMatrixInv) * positionRelPlanet;
	vec2 uv = cloudUvFromPosRelPlanet(positionRelPlanetPlanetAxes);
	float altitude = length(positionRelPlanet) - innerRadius;
	return sampleCloudShadowMaskAtCloudsUv(cloudSampler, uv, altitude, lightDirection);
}

float sampleCloudSkyOcclusionMaskAtCloudsUv(sampler2D cloudSampler, vec2 uv)
{
#ifdef USE_CLOUD_COVERAGE_MAP
	const int lod = 4; // use a lower res lod to get an average of occlusion in the area
	float coverage = clamp(textureLod(cloudSampler, uv, lod).r * 1.3 - 0.001, 0.0, 1.0);
#else
	float coverage = max(0.0, cloudCoverageFraction*1.5-0.5);
#endif
	return mix(1.0, 0.3, coverage);
}

float sampleCloudSkyOcclusionMaskAtPositionRelPlanet(sampler2D cloudSampler, vec3 positionRelPlanet)
{
	vec3 positionRelPlanetPlanetAxes = mat3(planetMatrixInv) * positionRelPlanet;
	vec2 uv = cloudUvFromPosRelPlanet(positionRelPlanetPlanetAxes);
	return sampleCloudSkyOcclusionMaskAtCloudsUv(cloudSampler, uv);
}

vec2 sampleCloudShadowAndSkyOcclusionMaskAtPositionRelPlanet(sampler2D cloudSampler, vec3 positionRelPlanet, vec3 lightDirection)
{
	vec3 positionRelPlanetPlanetAxes = mat3(planetMatrixInv) * positionRelPlanet;
	vec2 uv = cloudUvFromPosRelPlanet(positionRelPlanetPlanetAxes);
	float altitude = length(positionRelPlanet) - innerRadius;
	vec2 result;
	result.x = sampleCloudShadowMaskAtCloudsUv(cloudSampler, uv, altitude, lightDirection);
	result.y = sampleCloudSkyOcclusionMaskAtCloudsUv(cloudSampler, uv);
	return result;
}

#endif // CLOUD_SHADOWS_H