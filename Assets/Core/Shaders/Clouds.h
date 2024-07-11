/* Copyright Matthew Reid
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

vec4 sampleCoverageDetail(sampler2D coverageDetailSampler, vec2 uv, float lod)
{
	return pow(textureLod(coverageDetailSampler, uv.xy * vec2(300.0, 150.0), lod), vec4(1,3,2,1));
}

vec4 sampleBaseCloudCoverage(sampler2D cloudSampler, vec2 uv)
{
#ifdef USE_CLOUD_COVERAGE_MAP
	float c = clamp(textureLod(cloudSampler, uv, 0).r * 1.3 - 0.001, 0.0, 1.0);
	return vec4(pow(c, 0.45)); // apply fudge factor so that high detail cloud coverage matches map
#else
	return vec4(cloudCoverageFraction);
#endif
}

// Modulation a detail map by a coverage map. The modulation has the following behavior:
//   * Input coverage of 0 gives an output of 0.
//   * Input coverage of 1 gives an output of 1.
//   * Input coverage between 0 and 1 remaps the color range of the detail map
//     such that the mean output brightness equals coverage amount.
// @param filterWidth [0, 1] controls the smoothness of the output,
//    where 0 produces harsh edges and 1 smooths out the detail so much that 
//    the input coverage signal is passed through unchanged.
float coverageModulation(float coverage, float detail, float filterWidth)
{
	float f = 1.0 - coverage;
	return clamp(remapNormalized(detail*(1.0-filterWidth)+filterWidth, f, f+filterWidth), 0.0, 1.0);
}

vec4 coverageModulation(vec4 coverage, vec4 detail, vec4 filterWidth)
{
	vec4 f = vec4(1) - coverage;
	return clamp(remapNormalized(detail*(vec4(1)-filterWidth)+filterWidth, f, f+filterWidth), vec4(0), vec4(1));
}

vec4 cloudCoverageModulationFilterWidth = vec4(0.6);

struct DensityHullSample
{
	vec4 coverageBase;
	vec4 coverageDetail;
	vec4 heightMultiplier;
};

vec4 calcCloudDensityHull(DensityHullSample s)
{
	vec4 coverage = s.coverageBase * s.heightMultiplier;
	return coverageModulation(coverage, s.coverageDetail, cloudCoverageModulationFilterWidth);
}

// Calculates density hull eroded by a scale factor.
// scale of 1 gives no errosion. Scale of 0 gives total erosion.
vec4 calcCloudDensityErodedHull(DensityHullSample s, float scale)
{
	vec4 coverage = s.coverageBase * s.heightMultiplier;
	return coverageModulation(coverage, s.coverageDetail * scale, cloudCoverageModulationFilterWidth);
}

vec4 calcCloudDensityLowRes(DensityHullSample s)
{
	// Since the cloud shapes are contained within the hull,
	// we need to scale the coverage detail by this fudge factor so that the low res
	// clouds will have the same approximate shape as the high res clouds.
	float scale = 0.5;
	return calcCloudDensityErodedHull(s, scale);
}

#endif // CLOUDS_H