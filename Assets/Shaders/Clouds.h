/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef CLOUDS_H
#define CLOUDS_H

#pragma import_defines ( USE_CLOUD_COVERAGE_MAP )

#include "GlobalDefines.h"
#include "Remap.h"

uniform vec2 cloudDisplacementMeters;
uniform vec2 cloudUvScaleRelTerrainUv;
uniform vec2 cloudUvOffsetRelTerrainUv;
uniform float cloudCoverageFraction; // only used if USE_CLOUD_COVERAGE_MAP is not defined

vec2 offsetCloudUvByWorldDistance(vec2 startUv, vec2 worldDistance)
{
	return startUv + vec2(worldDistance.y / 40070000.0, 0.5 * worldDistance.x / 40070000.0);
}

vec2 cloudUvFromTerrainUv(vec2 geoTexCoord)
{
	vec2 uv = geoTexCoord.xy * cloudUvScaleRelTerrainUv + cloudUvOffsetRelTerrainUv;
	return offsetCloudUvByWorldDistance(uv, cloudDisplacementMeters);
}


vec2 cloudUvFromPosRelPlanet(vec3 positionRelPlanetPlanetAxes)
{
	vec2 pos2d = normalize(positionRelPlanetPlanetAxes.yx);
	float psi = atan(pos2d.x, pos2d.y);
	float theta = asin(normalize(positionRelPlanetPlanetAxes).z);
	vec2 uv = vec2(psi * M_RCP_2PI + 0.5, theta * M_RCP_PI + 0.5);
	return offsetCloudUvByWorldDistance(uv, cloudDisplacementMeters);
}

float sampleCoverageDetail(sampler2D coverageDetailSampler, vec2 uv, float lod, out float cloudType)
{
	vec4 coverageNoise = textureLod(coverageDetailSampler, uv.xy * vec2(160.0, 80.0), 4 * lod);
	float coverageDetail = coverageNoise.r;
	cloudType = coverageNoise.g;
	return coverageDetail;
}

float calcCloudDensityLowRes(sampler2D cloudSampler, vec2 uv, float heightMultiplier, float coverageDetail)
{
#ifdef USE_CLOUD_COVERAGE_MAP
	float coverageToDensityMultiplier = 5; // The is hand tuned to produce correct looking results.
	float density = max(0.0, remapNormalized(textureLod(cloudSampler, uv, 0).r, 0.05, 1.0)) * coverageToDensityMultiplier;
#else
	float coverageToDensityMultiplier = 2.5; // The is hand tuned to produce correct looking results.
	float density = cloudCoverageFraction * coverageToDensityMultiplier;
#endif
	density *= heightMultiplier;
	float coverageModulatedDensity = clamp(remapNormalized(coverageDetail, 1.0 - density, 1.0), 0.0, 1.0);
	
	// Return coverageModulatedDensity unless the unmodulated density is near zero, in which case
	// blend to unmodulated density to avoid artifacts from extreme remapping.
	return mix(density, coverageModulatedDensity, min(1.0, density * 10));
}


#endif // CLOUDS_H