/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#version 440 core
#pragma import_defines ( ENABLE_OCEAN )
#pragma import_defines ( ENABLE_ATMOSPHERE )

#include "AtmosphericScatteringWithClouds.h"
#include "CloudShadows.h"
#include "DepthPrecision.h"
#include "Ocean.h"
#include "Brdfs/BlinnPhong.h"
#include "Brdfs/Lambert.h"
#include "Util/Saturate.h"

in vec2 geoTexCoord;
in vec2 albedoTexCoord;
in vec3 normal;
in float logZ;
in vec3 positionRelCamera;
in AtmosphericScattering scattering;

out vec4 color;

uniform sampler2D albedoSampler;
uniform sampler2D landMaskSampler;
uniform sampler2D cloudSampler;
uniform vec3 lightDirection;
uniform vec2 heightMapUvScale;
uniform vec2 heightMapUvOffset;
uniform vec3 ambientLightColor;

void main()
{
	color = texture(albedoSampler, albedoTexCoord.xy);
	// No need to set gl_FragDepth here because the vertex shader is accurate enough
	// gl_FragDepth = logarithmicZ_fragmentShader(logZ);

	vec3 viewDirection = normalize(-positionRelCamera);
	vec3 waterColor = deepScatterColor;


	vec3 visibleSunIrradiance = scattering.sunIrradiance;
	
#ifdef ENABLE_OCEAN
	float landMask = texture(landMaskSampler, geoTexCoord.xy * heightMapUvScale + heightMapUvOffset).a;
	vec3 albedo = mix(waterColor, color.rgb, landMask);
	vec3 specularReflectance = oceanSpecularColor * (1.0 - landMask) * calcBlinnPhongSpecular(lightDirection, viewDirection, normal, oceanShininess) * visibleSunIrradiance;
#else
	vec3 albedo = color.rgb;
	vec3 specularReflectance = vec3(0);
#endif	

	vec3 totalReflectance = albedo * (
			calcLambertDirectionalLight(lightDirection, normal) * visibleSunIrradiance + 	
			calcLambertSkyLight(lightDirection, normal) * scattering.skyIrradiance +
			ambientLightColor
		)
		+ specularReflectance;
		
#ifdef ENABLE_ATMOSPHERE
	color.rgb = totalReflectance * scattering.transmittance + scattering.skyRadianceToPoint;
#else
	color.rgb = totalReflectance;
#endif

	color.rgb = saturate(color.rgb); // Saturate to avoid fireflies from specular

#ifdef DEBUG_EDGES

	if(any(lessThan(geoTexCoord.xy, vec2(0.02))))
	{
		color.rgb = vec3(1,0,0);
	}
#endif
}
