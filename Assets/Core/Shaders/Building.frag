/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#version 440 core
#pragma import_defines ( CAST_SHADOWS )
#pragma import_defines ( ENABLE_SHADOWS )

#include "AtmosphericScatteringWithClouds.h"
#include "DepthPrecision.h"
#include "Brdfs/BlinnPhong.h"
#include "Brdfs/Lambert.h"
#include "Shadows/Shadows.h"

in vec3 texCoord;
in vec3 normalWS;
in vec3 positionRelCamera;
in float logZ;
in AtmosphericScattering scattering;
in vec3 colorMultiplier;
in vec3 shadowTexCoord;

out vec4 color;

uniform sampler2DArray albedoSampler;
uniform vec3 lightDirection;
uniform vec3 ambientLightColor;

const float shininess = 32;
const float specularity = 0.3;

void main()
{
#ifdef CAST_SHADOWS
	return;
#endif

	color = texture(albedoSampler, texCoord.xyz);
	color.rgb *= colorMultiplier;

	float fragmentViewDistance = length(positionRelCamera);
	
#ifdef ENABLE_SHADOWS
	float dotLN = dot(lightDirection, normalWS);
	float lightVisibility = sampleShadowsAtTexCoord(shadowTexCoord.xy, shadowTexCoord.z, dotLN, fragmentViewDistance);
#else
	float lightVisibility = 1.0;
#endif
	
	vec3 visibleSunIrradiance = scattering.sunIrradiance * lightVisibility;
	
	color.rgb *= calcLambertDirectionalLight(lightDirection, normalWS) * visibleSunIrradiance
		+ calcLambertSkyLight(lightDirection, normalWS) * scattering.skyIrradiance
		+ ambientLightColor;
	
	vec3 viewDirection = -positionRelCamera / fragmentViewDistance;
	color.rgb += visibleSunIrradiance * specularity * vec3(calcBlinnPhongSpecular(lightDirection, viewDirection, normalWS, shininess));

	color.rgb = color.rgb * scattering.transmittance + scattering.skyRadianceToPoint;
	
	gl_FragDepth = logarithmicZ_fragmentShader(logZ);
}
