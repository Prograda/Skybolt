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
	float coverageDetail = sampleCoverageDetail(coverageDetailSampler2, uv, /* lod */ 0.0, cloudType);
	float density = calcCloudDensityLowRes(cloudSampler, uv, /* heightMultiplier */ 1.0, coverageDetail);
	return clamp(remap(density, 0.5, 0.57, 0.0, 1.0), 0.0, 1.0); // should look the same as smallest mipmap of the baseNoiseSampler texture
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
	//return texture(cloudSampler, offsetCloudUvByWorldDistance(uv, cloudProjectionWorldDistance)).r;
	return 0.1 * calcDensityLowRes2(cloudSampler, offsetCloudUvByWorldDistance(uv, cloudProjectionWorldDistance));
}

float sampleCloudShadowMaskAtPositionRelPlanet(sampler2D cloudSampler, vec3 positionRelPlanet, vec3 lightDirection)
{
	vec3 positionRelPlanetPlanetAxes = mat3(planetMatrixInv) * positionRelPlanet;
	vec2 uv = cloudUvFromPosRelPlanet(positionRelPlanetPlanetAxes);
	float mask = sampleCloudShadowMask(cloudSampler, uv, lightDirection);
	float altitude = -positionRelPlanet.z - innerRadius;
	return mix(mask, 1.0, clamp(remapNormalized(altitude, 2000, 5000), 0.0, 1.0)); // TODO: use actual cloud heights
}

#endif // CLOUD_SHADOWS_H