/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef PRINCIPLED_BRDF_ENVIRONMENT_LIGHT_H
#define PRINCIPLED_BRDF_ENVIRONMENT_LIGHT_H

#include "EnvironmentMap.h"
#include "Brdfs/Lambert.h"
#include "Util/Specular.h"

struct PrincipledBrdfEnvironmentLightResult
{
	vec3 diffuse;
#ifdef ENABLE_SPECULAR
	vec3 specular;
#endif
};

struct PrincipledBrdfEnvironmentLightArgs
{
	float dotNV;
	vec3 lightDirection;
	vec3 normal;
	vec3 viewDirection;
	
	// TODO: use the environment sampler instead of groundIrradiance and skyIrradiance
	vec3 groundIrradiance;
	vec3 skyIrradiance;
#ifdef ENABLE_SPECULAR
	vec3 specularF0;
#endif
	float roughness;
};

PrincipledBrdfEnvironmentLightResult evalPrincipledBrdfEnvironmentLight(PrincipledBrdfEnvironmentLightArgs args, sampler2D environmentSampler)
{
	PrincipledBrdfEnvironmentLightResult result;

	result.diffuse = calcLambertAmbientLight(args.normal, args.groundIrradiance, args.skyIrradiance);

#ifdef ENABLE_SPECULAR
	#ifdef ENABLE_ENVIRONMENT_MAP
		vec3 reflectionDir = reflect(-args.viewDirection, args.normal);
		vec2 environmentUv = getSphericalEnvironmentMapTexCoord(reflectionDir);
		float lod = max(args.roughness * 10, textureQueryLod(environmentSampler, environmentUv).x);

		vec3 Fenv = fresnelSchlickRoughness(args.dotNV, args.specularF0, roughnessToGlossiness(args.roughness));
		
		result.diffuse *= vec3(1) - Fenv;
		result.specular = textureLod(environmentSampler, environmentUv, lod).rgb * Fenv;
	#endif
#endif
	return result;
}
	
#endif // PRINCIPLED_BRDF_ENVIRONMENT_LIGHT_H
