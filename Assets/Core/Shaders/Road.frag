/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#version 440 core

#pragma import_defines ( ENABLE_ATMOSPHERE )
#pragma import_defines ( ENABLE_DEPTH_OFFSET )
#pragma import_defines ( ENABLE_SHADOWS )

#include "AtmosphericScatteringWithClouds.h"
#include "DepthPrecision.h"
#include "GroundIrradiance.h"
#include "NormalMapping.h"
#include "Brdfs/BlinnPhong.h"
#include "Brdfs/Lambert.h"
#include "Shadows/Shadows.h"
#include "Util/Saturate.h"

in vec3 texCoord;
in vec3 normalWS;
in float logZ;
in vec3 positionRelCamera;
in vec3 shadowTexCoord;
in AtmosphericScattering scattering;

out vec4 color;

uniform sampler2D albedoSampler;
uniform sampler2D markingsSampler;

uniform vec3 lightDirection;
uniform vec3 ambientLightColor;

#ifdef ENABLE_DEPTH_OFFSET
	uniform float depthOffset;
#endif

void main()
{
	color = texture(albedoSampler, texCoord.xy);
	vec4 markings = texture(markingsSampler, vec2(texCoord.x*2, texCoord.y));
	
	bool isEdge = abs(texCoord.z - 0.5) >  0.37;
	float marking = isEdge ? 0 : markings.g;
	color = mix(color, vec4(1), marking);
	
	float fragmentViewDistance = length(positionRelCamera);

#ifdef ENABLE_SHADOWS
	float dotLN = dot(lightDirection, normalWS);
	float lightVisibility = sampleShadowsAtTexCoord(shadowTexCoord.xy, shadowTexCoord.z, dotLN, fragmentViewDistance);
#else
	float lightVisibility = 1.0;
#endif
	vec3 viewDirection = -positionRelCamera/fragmentViewDistance;
	
	color.rgb *= calcLambertDirectionalLight(lightDirection, normalWS) * scattering.sunIrradiance * lightVisibility
#ifdef ENABLE_ATMOSPHERE
		+ calcLambertAmbientLight(normalWS, calcGroundIrradiance(scattering.sunIrradiance, scattering.skyIrradiance, lightDirection), scattering.skyIrradiance)
#endif
		+ ambientLightColor;

//#define ENABLE_SPECULAR
#ifdef ENABLE_SPECULAR
	color.rgb += 0.03 * calcBlinnPhongSpecular(lightDirection, viewDirection, normalWS, 10) * scattering.sunIrradiance;
#endif

#ifdef ENABLE_ATMOSPHERE
	color.rgb = color.rgb * scattering.transmittance + scattering.skyRadianceToPoint;
#endif

	color.rgb = saturate(color.rgb); // clamp to avoid fireflies from specular

	gl_FragDepth = logarithmicZ_fragmentShader(logZ);

#ifdef ENABLE_DEPTH_OFFSET
	gl_FragDepth += depthOffset;
#endif
}
