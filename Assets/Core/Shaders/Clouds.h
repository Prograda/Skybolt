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
	vec4 coverageNoise = textureLod(coverageDetailSampler, uv.xy * vec2(200.0, 100.0), 4 * lod);
	float coverageDetail = coverageNoise.r;
	cloudType = coverageNoise.g;
	return coverageDetail;
}

float calcCloudDensityLowRes(sampler2D cloudSampler, vec2 uv, float heightMultiplier, float coverageDetail)
{
	float coverage = heightMultiplier;
#ifdef USE_CLOUD_COVERAGE_MAP
	coverage *= max(0.0, remapNormalized(textureLod(cloudSampler, uv, 0).r, 0.05, 1.0));
#else
	float coverageFractionMultiplier = 0.8; // scale coverage fraction to give more user friendly results
	coverage *= cloudCoverageFraction * coverageFractionMultiplier;
#endif
	// Apply coverage detail map by remapping with the following behavior:
	//   Input coverage of 0 gives an output coverage of 0.
	//   Input coverage of 1 gives an output coverage of 1.
	//   Input coverage between 0 and 1 returns a color band with within the detail map,
	//   where the band contains values of the expected output coverage amount.
	//   the band is sized using filterWidth to give a enough coverage variation to achieve smooth density changes.
	float f = 1.0 - coverage;
	float filterWidth = 0.2;
	return clamp(remapNormalized(coverageDetail*(1.0-filterWidth)+filterWidth, f, f+filterWidth), 0.0, 1.0);
}


#endif // CLOUDS_H