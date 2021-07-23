/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#version 440 core

#pragma import_defines ( ENABLE_ATMOSPHERE )
#pragma import_defines ( ENABLE_DEPTH_OFFSET )
#pragma import_defines ( ENABLE_ENVIRONMENT_MAP )
#pragma import_defines ( ENABLE_NORMAL_MAP )
#pragma import_defines ( ENABLE_SHADOWS )
#pragma import_defines ( ENABLE_SPECULAR )

#include "AtmosphericScatteringWithClouds.h"
#include "DepthPrecision.h"
#include "EnvironmentMap.h"
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
in AtmosphericScattering scattering;

out vec4 color;

uniform sampler2D albedoSampler;
uniform sampler2D normalSampler;

uniform vec3 lightDirection;
uniform vec3 ambientLightColor;
uniform vec3 specularity;
uniform float specularExponent;

uniform sampler2D environmentSampler;

#ifdef ENABLE_DEPTH_OFFSET
	uniform float depthOffset;
#endif

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness) {
	// Fresnel approximation that considers roughness.
	// Inspired by https://seblagarde.wordpress.com/2011/08/17/hello-world/
	// but tweaked to make roughness occlude more.
	// TODO: need to come up with something better
	float fresnelOcclusion = 1.0-pow(roughness, 0.2);
    return F0 + fresnelOcclusion * (vec3(1.0) - F0) * pow(1.0 - cosTheta, 5.0);
}

// Converts from Blinn-Phong 'shininess' (specular exponent) to Beckmann roughness
// From http://simonstechblog.blogspot.com/2011/12/microfacet-brdf.html
float specularExponentToRoughness(float exponent)
{
	return sqrt(2 / (exponent + 2));
}

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
	vec3 normal = normalize(normalWS);
#endif
	
#ifdef ENABLE_SHADOWS
	float dotLN = dot(lightDirection, normal);
	float lightVisibility = sampleShadowsAtTexCoord(shadowTexCoord.xy, shadowTexCoord.z, dotLN, fragmentViewDistance);
#else
	float lightVisibility = 1.0;
#endif
	vec3 viewDirection = -positionRelCamera / fragmentViewDistance;
	
	vec3 visibleSunIrradiance = scattering.sunIrradiance * lightVisibility;
	
	color.rgb *= calcLambertDirectionalLight(lightDirection, normal) * visibleSunIrradiance
	//color.rgb *= calcOrenNayerDirectionalLight(lightDirection, viewDirection, normal, 0.8) * scattering.sunIrradiance * lightVisibility
#ifdef ENABLE_ATMOSPHERE
		+ calcLambertAmbientLight(normal, scattering.sunIrradiance, scattering.skyIrradiance)
#endif
		+ ambientLightColor;

#ifdef ENABLE_SPECULAR

	float NdotV = max(0.0, dot(normal, viewDirection));
	float roughness = specularExponentToRoughness(specularExponent);
	vec3 F = fresnelSchlickRoughness(NdotV, specularity, roughness);
    vec3 kS = F;
	vec3 kD = vec3(1.0) - kS;

	vec3 specular = calcBlinnPhongSpecular(lightDirection, viewDirection, normal, specularExponent) * visibleSunIrradiance;
		
	#ifdef ENABLE_ENVIRONMENT_MAP
		vec3 reflectionDir = reflect(-viewDirection, normal);
		vec2 environmentUv = getSphericalEnvironmentMapTexCoord(reflectionDir);
		float lod = max(roughness * 10, textureQueryLod(environmentSampler, environmentUv).x);
		specular += textureLod(environmentSampler, environmentUv, lod).rgb;
	#endif
	
	color.rgb = color.rgb * kD + specular * kS;
#endif

#ifdef ENABLE_ATMOSPHERE
	color.rgb = color.rgb * scattering.transmittance + scattering.skyRadianceToPoint;
#endif
	gl_FragDepth = logarithmicZ_fragmentShader(logZ);

#ifdef ENABLE_DEPTH_OFFSET
	gl_FragDepth += depthOffset;
#endif
}
