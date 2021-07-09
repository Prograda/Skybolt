/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#version 440 core

#pragma import_defines ( ENABLE_ATMOSPHERE )
#pragma import_defines ( ENABLE_DEPTH_OFFSET )
#pragma import_defines ( ENABLE_NORMAL_MAP )
#pragma import_defines ( ENABLE_SHADOWS )

#include "DepthPrecision.h"
#include "NormalMapping.h"
#include "Brdfs/BlinnPhong.h"
#include "Brdfs/Lambert.h"
#include "Brdfs/OrenNayer.h"
#include "Shadows/Shadows.h"

in vec3 texCoord;
in vec3 normalWS;
in vec3 tangentWS;
in float logZ;
in vec3 sunIrradiance;
in vec3 positionRelCamera;
in vec3 shadowTexCoord;

#ifdef ENABLE_ATMOSPHERE
	in vec3 skyIrradiance;
	in vec3 transmittance;
	in vec3 skyRadianceToPoint;
#endif

out vec4 color;

uniform sampler2D albedoSampler;
uniform sampler2D normalSampler;

uniform vec3 lightDirection;
uniform vec3 ambientLightColor;

#ifdef ENABLE_DEPTH_OFFSET
	uniform float depthOffset;
#endif

void main()
{
	color = texture(albedoSampler, texCoord.xy);
	
	float fragmentViewDistance = length(positionRelCamera);

#ifdef ENABLE_NORMAL_MAP
	vec3 normal = texture(normalSampler, texCoord.xy).rgb;
	normal = normalize(normal * 2.0 - 1.0);
	mat3 tbn = getTbn(tangentWS, normalWS);
	normal = normalize(tbn * normal); 
#else
	vec3 normal = normalWS;
#endif
	
#ifdef ENABLE_SHADOWS
	float dotLN = dot(lightDirection, normal);
	float lightVisibility = sampleShadowsAtTexCoord(shadowTexCoord.xy, shadowTexCoord.z, dotLN, fragmentViewDistance);
#else
	float lightVisibility = 1.0;
#endif
	vec3 viewDirection = -positionRelCamera/fragmentViewDistance;
	
	color.rgb *= calcLambertDirectionalLight(lightDirection, normal) * sunIrradiance * lightVisibility
	//color.rgb *= calcOrenNayerDirectionalLight(lightDirection, viewDirection, normal, 0.8) * sunIrradiance * lightVisibility
#ifdef ENABLE_ATMOSPHERE
		+ calcLambertAmbientLight(normal, sunIrradiance, skyIrradiance)
#endif
		+ ambientLightColor;

//#define ENABLE_SPECULAR
#ifdef ENABLE_SPECULAR
	color.rgb += 0.03 * calcBlinnPhongSpecular(lightDirection, viewDirection, normal, 10) * sunIrradiance;
#endif

#ifdef ENABLE_ATMOSPHERE
	color.rgb = color.rgb * transmittance + skyRadianceToPoint;
#endif
	gl_FragDepth = logarithmicZ_fragmentShader(logZ);

#ifdef ENABLE_DEPTH_OFFSET
	gl_FragDepth += depthOffset;
#endif
}
