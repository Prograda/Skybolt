/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#version 440 core

#pragma import_defines ( CAST_SHADOWS )
#pragma import_defines ( ENABLE_ATMOSPHERE )
#pragma import_defines ( ENABLE_DEPTH_OFFSET )
#pragma import_defines ( ENABLE_ENVIRONMENT_MAP )
#pragma import_defines ( ENABLE_NORMAL_MAP )
#pragma import_defines ( ENABLE_SPECULAR_MAP )
#pragma import_defines ( ENABLE_ROUGHNESS_MAP )
#pragma import_defines ( ENABLE_OCCLUSION_ROUGHNESS_METALNESS_MAP )
#pragma import_defines ( ENABLE_SHADOWS )
#pragma import_defines ( ENABLE_SPECULAR )
#pragma import_defines ( UNIFORM_ALBEDO )

#include "AtmosphericScatteringWithClouds.h"
#include "DepthPrecision.h"
#include "GroundIrradiance.h"
#include "NormalMapping.h"
#include "Brdfs/PrincipledBrdfDirectionalLight.h"
#include "Brdfs/PrincipledBrdfEnvironmentLight.h"
#include "Shadows/Shadows.h"

in vec3 texCoord;
in vec3 normalWS;
in vec3 tangentWS;
in float logZ;
in vec3 positionRelCamera;
in vec3 shadowTexCoord;
in AtmosphericScattering scattering;

out vec4 color;

uniform vec3 lightDirection;
uniform vec3 ambientLightColor;
uniform vec3 specularity;
uniform float roughness;

#ifdef UNIFORM_ALBEDO
	uniform vec4 albedoColor;
#else
	uniform sampler2D albedoSampler;
#endif

uniform sampler2D normalSampler;
uniform sampler2D specularSampler;
uniform sampler2D roughnessSampler;
uniform sampler2D occlusionRoughnessMetalnessSampler; // TODO: support Metalness
uniform sampler2D environmentSampler;

const float roughnessMapPower = 0.35;

#ifdef ENABLE_DEPTH_OFFSET
	uniform float depthOffset;
#endif

void main()
{
#ifdef CAST_SHADOWS
	return;
#endif
	float fragmentViewDistance = length(positionRelCamera);

#ifdef ENABLE_NORMAL_MAP
	vec3 normal = texture(normalSampler, texCoord.xy).rgb;
	normal = normalize(normal * 2.0 - 1.0);
	mat3 tbn = getTbn(tangentWS, normalWS);
	normal = normalize(tbn * normal); 
#else
	vec3 normal = normalize(normalWS);
#endif

	vec3 viewDirection = -positionRelCamera / fragmentViewDistance;
	float dotNL = saturate(dot(normal, lightDirection));
	float dotNV = saturate(dot(normal, viewDirection));

#ifdef ENABLE_SPECULAR_MAP
	vec3 sampledSpecularity = texture(specularSampler, texCoord.xy).rgb * 0.08; // specular rescaled by 0.08 as per Disney BRDF
#else
	vec3 sampledSpecularity = specularity;
#endif

#ifdef ENABLE_ROUGHNESS_MAP
	float sampledRoughness = texture(roughnessSampler, texCoord.xy).r;
#else
#ifdef ENABLE_OCCLUSION_ROUGHNESS_METALNESS_MAP
	vec4 occlusionRoughnessMetalness = texture(occlusionRoughnessMetalnessSampler, texCoord.xy);
	float sampledRoughness = pow(occlusionRoughnessMetalness.g, roughnessMapPower);
#else
	float sampledRoughness = roughness;
#endif
#endif

#ifdef ENABLE_SHADOWS
	float lightVisibility = sampleShadowsAtTexCoord(shadowTexCoord.xy, shadowTexCoord.z, dotNL, fragmentViewDistance);
#else
	float lightVisibility = 1.0;
#endif

	vec3 visibleSunIrradiance = scattering.sunIrradiance * lightVisibility;

	PrincipledBrdfDirectionalLightResult directResult;
	{
		PrincipledBrdfDirectionalLightArgs args;
		
		args.dotNL = dotNL;
		args.lightDirection = lightDirection;
		args.normal = normal;
		args.viewDirection = viewDirection;
		args.irradiance = visibleSunIrradiance;
	#ifdef ENABLE_SPECULAR
		args.specularF0 = sampledSpecularity;
	#endif
		args.roughness = sampledRoughness;
		
		directResult = evalPrincipledBrdfDirectionalLight(args);
	}
	
	PrincipledBrdfEnvironmentLightResult environmentResult;
	{
		PrincipledBrdfEnvironmentLightArgs args;
		
		args.dotNV = dotNV;
		args.normal = normal;
		args.viewDirection = viewDirection;
		args.groundIrradiance = calcGroundIrradiance(scattering.sunIrradiance, scattering.skyIrradiance, lightDirection);
		args.skyIrradiance = scattering.skyIrradiance;

	#ifdef ENABLE_SPECULAR
		args.specularF0 = sampledSpecularity;
	#endif
		args.roughness = sampledRoughness;
		
		environmentResult = evalPrincipledBrdfEnvironmentLight(args, environmentSampler);

	#ifdef ENABLE_OCCLUSION_ROUGHNESS_METALNESS_MAP
		// Apply occlusion
		environmentResult.diffuse *= occlusionRoughnessMetalness.r;
		environmentResult.specular *= occlusionRoughnessMetalness.r;
	#endif
	}

#ifdef UNIFORM_ALBEDO
	vec4 albedo = albedoColor;
#else
	vec4 albedo = texture(albedoSampler, texCoord.xy);
#endif
	color.a = albedo.a;

	color.rgb = albedo.rgb * (directResult.diffuse + environmentResult.diffuse + ambientLightColor);
#ifdef ENABLE_SPECULAR
	color.rgb += directResult.specular + environmentResult.specular;
#endif

#ifdef ENABLE_ATMOSPHERE
	color.rgb = color.rgb * scattering.transmittance + scattering.skyRadianceToPoint;
#endif
	gl_FragDepth = logarithmicZ_fragmentShader(logZ);

#ifdef ENABLE_DEPTH_OFFSET
	gl_FragDepth += depthOffset;
#endif
}
