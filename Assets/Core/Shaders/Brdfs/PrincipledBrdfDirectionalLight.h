/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef PRINCIPLED_BRDF_DIRECTIONAL_LIGHT_H
#define PRINCIPLED_BRDF_DIRECTIONAL_LIGHT_H

#include "Brdfs/Ggx.h"
#include "Brdfs/Lambert.h"
#include "Util/Saturate.h"
#include "Util/Specular.h"

struct PrincipledBrdfDirectionalLightResult
{
	vec3 diffuse;
#ifdef ENABLE_SPECULAR
	vec3 specular;
#endif
};

struct PrincipledBrdfDirectionalLightArgs
{
	float dotNL;
	vec3 lightDirection;
	vec3 normal;
	vec3 viewDirection;
	vec3 irradiance;
#ifdef ENABLE_SPECULAR
	vec3 specularF0;
#endif
	float roughness;
};

PrincipledBrdfDirectionalLightResult evalPrincipledBrdfDirectionalLight(PrincipledBrdfDirectionalLightArgs args)
{
	PrincipledBrdfDirectionalLightResult result;
	
	// Note: This could be optimized by deriving dotNH and dotLH from identities as per
	// https://ubm-twvideo01.s3.amazonaws.com/o1/vault/gdc2017/Presentations/Hammon_Earl_PBR_Diffuse_Lighting.pdf
	vec3 H = calcHalfAngle(args.viewDirection, args.lightDirection);
	float dotNH = saturate(dot(args.normal, H));
	float dotLH = saturate(dot(args.lightDirection, H));
	
	result.diffuse = calcLambertDirectionalLight(args.lightDirection, args.normal) * args.irradiance;

#ifdef ENABLE_SPECULAR
	vec3 F = fresnelSchlick(dotLH, vec3(args.specularF0));
	result.diffuse *= vec3(1.0) - F;

	float alpha = args.roughness * args.roughness;
	float distribution = calcGgxD(dotNH, alpha);
	
	//float specularVisibility = calcGgxV(result.dotNL, dotNV, alpha); // exact
	float specularVisibility = calcGgxVApprox(dotLH, alpha);

	float microfacetNormalizationFactor = 0.25;
	result.specular = args.dotNL * distribution * F * specularVisibility * args.irradiance * microfacetNormalizationFactor;
#endif
	return result;
}
	
#endif // PRINCIPLED_BRDF_DIRECTIONAL_LIGHT_H
