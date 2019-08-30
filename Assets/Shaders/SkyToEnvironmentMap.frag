/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#version 440 core
#pragma import_defines ( ENABLE_CLOUDS )

#include "AtmosphericScattering.h"
#include "CloudShadows.h"
#include "GlobalDefines.h"

in vec3 texCoord;

out vec4 color;

uniform vec3 cameraPosition;
uniform vec3 lightDirection;
uniform sampler2D cloudSampler;

void main()
{
	// Get the ray from the camera to the vertex, and its length (which is the far point of the ray passing through the atmosphere)
	vec3 cameraPositionRelPlanetCenter = cameraPosition - planetCenter;

    vec2 u = (texCoord.xy * 2.0 - vec2(1.0)) * 1.1;
    float l = dot(u, u);

    if (l <= 1.02)
	{
        if (l > 1.0)
		{
            u = u / l;
            l = 1.0 / l;
        }

        // inverse stereographic projection,
        // from skymap coordinates to world space directions
        vec3 rayDir = vec3(2.0 * u, 1.0 - l) / (1.0 + l);
	
		rayDir = -rayDir;
		
		vec3 transmittance;
		color.rgb = GetSkyRadiance(cameraPositionRelPlanetCenter, rayDir, 0, lightDirection, transmittance);

#ifdef ENABLE_CLOUDS
		float cloudAlpha = sampleCloudAlphaAtPositionRelPlanet(cloudSampler, cameraPositionRelPlanetCenter, -rayDir);
		color.rgb = mix(color.rgb, vec3(0.7), cloudAlpha);
#endif
    }
	else
	{
        // below horizon:
        // use average fresnel * average sky radiance
        // to simulate multiple reflections on waves

        const float avgFresnel = 0.17;
		vec3 skyIrradiance;
		vec3 sunIrradiance = GetSunAndSkyIrradiance(cameraPositionRelPlanetCenter, lightDirection, skyIrradiance);
		
        color.rgb = skyIrradiance / M_PI * avgFresnel;
    }

	color.a = 1;
}
