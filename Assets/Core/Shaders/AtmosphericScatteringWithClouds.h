/* Copyright 2012-2020 Matthew Reid
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
    vec3 lightDirection,
	sampler2D cloudSampler) {

	AtmosphericScattering OUT;
	
	float shadowLength = 0.0; // length of shadowed part of view ray
	OUT.skyRadianceToPoint = GetSkyRadianceToPoint(cameraPositionRelPlanet, positionRelPlanet, shadowLength, lightDirection, OUT.transmittance);
	OUT.sunIrradiance = GetSunAndSkyIrradiance(positionRelPlanet, lightDirection, OUT.skyIrradiance);
	
	// Adjust scattering based on cloud coverage.
	// Heaver cloud coverage decreases irradiance and inscattering.
#ifdef ENABLE_CLOUDS
	vec2 mask = sampleCloudShadowAndSkyOcclusionMaskAtPositionRelPlanet(cloudSampler, positionRelPlanet, lightDirection);
	float shadowMask = mask.x;
	float occlusionMask = mask.y;
	
	const float hemisphereFraction = 0.5; // ignore the fraction of the irradiance that is scattered back up
	float cloudTransmission = occlusionMask;
	vec3 overcastSkyIrradiance = (OUT.sunIrradiance + OUT.skyIrradiance) * hemisphereFraction * cloudTransmission;

	OUT.sunIrradiance *= shadowMask;
	OUT.skyIrradiance = mix(overcastSkyIrradiance, OUT.skyIrradiance, occlusionMask);
	OUT.skyRadianceToPoint *= occlusionMask;
#endif
	
	return OUT;
}

#endif // ATMOSPHERIC_SCATTERING_WITH_CLOUDS_H
