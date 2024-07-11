/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef ATMOSPHERIC_SCATTERING_WITH_CLOUDS_H
#define ATMOSPHERIC_SCATTERING_WITH_CLOUDS_H

#pragma import_defines ( ENABLE_CLOUDS )

#include "AtmosphericScattering.h"
#include "CloudShadows.h"

struct AtmosphericScattering
{
	vec3 skyRadianceToPoint;
	vec3 transmittance;
	vec3 sunIrradiance;
	vec3 skyIrradiance;
};

AtmosphericScattering calcAtmosphericScattering(
    vec3 cameraPositionRelPlanet,
    vec3 positionRelPlanet,
    vec3 lightDirection) {

	float shadowLength = 0.0; // length of shadowed part of view ray

	AtmosphericScattering S;
	S.sunIrradiance = GetSunAndSkyIrradiance(positionRelPlanet, lightDirection, S.skyIrradiance);
	S.skyRadianceToPoint = GetSkyRadianceToPoint(cameraPositionRelPlanet, positionRelPlanet, shadowLength, lightDirection, S.transmittance);
	
	return S;
}

AtmosphericScattering applyCloudOcclusion(
	AtmosphericScattering S,
    vec3 cameraPositionRelPlanet,
    vec3 positionRelPlanet,
    vec3 lightDirection,
	sampler2D cloudSampler) {

	// Adjust scattering based on cloud coverage.
	// Heaver cloud coverage decreases irradiance and inscattering.
#ifdef ENABLE_CLOUDS
	vec2 mask = sampleCloudShadowAndSkyOcclusionMaskAtPositionRelPlanet(cloudSampler, positionRelPlanet, lightDirection);
	float shadowMask = mask.x;
	float occlusionMask = mask.y;

	vec3 cameraToPointDir = positionRelPlanet - cameraPositionRelPlanet;
	float cameraToPointDistance = length(cameraToPointDir);
	cameraToPointDir /= cameraToPointDistance;

	// Fade out occlusion with distance to simulate increased inscattering as distance increases
	// due larger un-occluded portion of view ray
	float skyRadianceOcclusionMask = mix(occlusionMask, 1.0, min(1.0, cameraToPointDistance/500000));

	S.skyRadianceToPoint *= skyRadianceOcclusionMask;
	S.sunIrradiance *= shadowMask;
	S.skyIrradiance *= occlusionMask;
#endif
	
	return S;
}

AtmosphericScattering calcAtmosphericScattering(
    vec3 cameraPositionRelPlanet,
    vec3 positionRelPlanet,
    vec3 lightDirection,
	sampler2D cloudSampler) {

	AtmosphericScattering S = calcAtmosphericScattering(cameraPositionRelPlanet, positionRelPlanet, lightDirection);
	S = applyCloudOcclusion(S, cameraPositionRelPlanet, positionRelPlanet, lightDirection, cloudSampler);
	return S;
}

RadianceSpectrum GetSkyRadianceToPointWithCloudOcclusion(
	Position camera, Position point, Direction lightDirection, float cloudOcclusionMask, out DimensionlessSpectrum transmittance)
{
	vec3 cameraToPointDir = point - camera;
	float cameraToPointDistance = length(cameraToPointDir);
	cameraToPointDir /= cameraToPointDistance;

	// Fade out occlusion with distance to simulate increased inscattering as distance increases
	// due larger un-occluded portion of view ray
	cloudOcclusionMask = mix(cloudOcclusionMask, 1.0, min(1.0, cameraToPointDistance/500000));

	float shadowLength = 0.0;
	return cloudOcclusionMask * GetSkyRadianceToPoint(camera, point, cameraToPointDir, shadowLength, lightDirection, transmittance);
}

#endif // ATMOSPHERIC_SCATTERING_WITH_CLOUDS_H
