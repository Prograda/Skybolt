/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#version 440 core
#pragma import_defines ( CAST_SHADOWS )
#pragma import_defines ( ENABLE_ATMOSPHERE )
#pragma import_defines ( ENABLE_OCEAN )
#pragma import_defines ( ENABLE_SHADOWS )
#pragma import_defines ( DETAIL_MAPPING_TECHNIQUE_ALBEDO_DERIVED )
#pragma import_defines ( DETAIL_MAPPING_TECHNIQUE_UNIFORM )
#pragma import_defines ( DETAIL_SAMPLER_COUNT )

#include "AtmosphericScatteringWithClouds.h"
#include "DepthPrecision.h"
#include "NormalMapping.h"
#include "Ocean.h"
#include "Rerange.h"
#include "Remap.h"
#include "Brdfs/BlinnPhong.h"
#include "Brdfs/Lambert.h"
#include "Noise/Fbm.h"
#include "Shadows/Shadows.h"
#include "Util/Srgb.h"

in vec3 texCoord;
in vec3 position_worldSpace;
in vec3 wrappedNoiseCoord;
in float cameraDistance;
in float elevation;
in AtmosphericScattering scatteringPerVertex;
in float logZ;

out vec4 color;

uniform sampler2D normalSampler;
uniform sampler2D landMaskSampler;
uniform sampler2D attributeSampler;
#ifdef DETAIL_SAMPLER_COUNT
	uniform sampler2D albedoDetailSamplers[DETAIL_SAMPLER_COUNT];
#endif
uniform sampler2D overallAlbedoSampler;
uniform sampler2D cloudSampler;
uniform sampler2D noiseSampler;

uniform vec2 heightMapUvScale;
uniform vec2 heightMapUvOffset;
uniform vec2 overallAlbedoMapUvScale;
uniform vec2 overallAlbedoMapUvOffset;
uniform vec2 attributeMapUvScale;
uniform vec2 attributeMapUvOffset;
uniform vec3 lightDirection;
uniform vec3 cameraPosition;
uniform vec3 ambientLightColor;

const float detailAlbedoUvScale[4] = float[4](0.01, 0.01, 0.01, 0.01);
const float detailAlbedoBrightnessScale[4] = float[4](1, 1, 1, 1);

ivec4 bilinearFetchIndices(sampler2D tex, vec2 uv, out vec4 weights)
{
	// TODO: consider clamping at edges here
	ivec2 dims = textureSize(tex, 0);
	vec2 coordFloat = uv * vec2(dims) - 0.5f;
	ivec2 coordInt = ivec2(floor(coordFloat));
	vec2 weight = coordFloat - coordInt;
	coordInt = clamp(coordInt, ivec2(0), dims-ivec2(2));
	
	float c00 = texelFetch(tex, coordInt, 0).r;
	float c10 = texelFetch(tex, coordInt + ivec2(1, 0), 0).r;
	float c01 = texelFetch(tex, coordInt + ivec2(0, 1), 0).r;
	float c11 = texelFetch(tex, coordInt + ivec2(1, 1), 0).r;

	weights.x = (1-weight.x) * (1-weight.y);
	weights.y = weight.x * (1-weight.y);
	weights.z = (1-weight.x) * weight.y;
	weights.w = weight.x * weight.y;
	
	return ivec4(c00*255.f + 0.5f, c10*255.f + 0.5f, c01*255.f + 0.5f, c11*255.f + 0.5f);
}

float saturation(vec3 c)
{
	float minColor = min(min(c.r, c.g), c.b);
	float maxColor = max(max(c.r, c.g), c.b);
	return (maxColor - minColor) / (maxColor + minColor);
}

float luminance(vec3 c)
{
	return dot(c, vec3(0.2126, 0.7152, 0.0722));
}

ivec4 bilinearFetchIndicesFromAlbedo(vec3 albedo, out vec4 weights)
{
	ivec4 indices = ivec4(0, 1, 2, 3);
	
	float grass = clamp((albedo.g / (albedo.r + albedo.b) - 0.5) * 3.0, 0, 1);
	float forestConditionalOnGrass = clamp((albedo.g - 0.1) * 5, 0, 1);
	
	weights = vec4(0);
	weights.x = 1.0;
	weights.y = grass;
	weights.z = grass * forestConditionalOnGrass;
	return indices;
}

vec4 blend(vec4 texture1, float a1, vec4 texture2, float a2)  
{  
    float depth = 0.2;
    float ma = max(texture1.a + a1, texture2.a + a2) - depth;  
  
    float b1 = max(texture1.a + a1 - ma, 0);  
    float b2 = max(texture2.a + a2 - ma, 0);  
  
    return (texture1 * b1 + texture2 * b2) / (b1 + b2);  
}

vec3 blendErosion(vec3 base, vec3 blend, vec3 falloff)
{
	vec3 invBase = (vec3(1) - base) * (vec3(1.0) + falloff);
	return clamp(remapNormalized(blend, vec3(invBase-falloff), vec3(invBase)), vec3(0), vec3(1));
}

#ifdef DETAIL_SAMPLER_COUNT
vec4 sampleDetailAlbedo(int i, vec3 normal, vec3 texCoord, vec2 normalUv)
{
	return texture(albedoDetailSamplers[i], texCoord.xy);
}

vec4 sampleDetailAlbedoMultiScale(int i, vec3 normal, vec3 texCoord, vec2 normalUv)
{
	vec4 c = texture(albedoDetailSamplers[i], texCoord.xy);
	vec4 d = texture(albedoDetailSamplers[i], texCoord.xy * 4);
	
	float lod = textureQueryLod(albedoDetailSamplers[i], texCoord.xy).r;
	
	return mix(c, d, clamp(1-lod*0.5, 0, 1));
}

vec4 sampleDetailTexture()
{
	vec3 detailTexCoordPerMeter = wrappedNoiseCoord * 10000.0; // repeats [0,1) per meter
	vec3 texCoord = detailAlbedoUvScale[0] * detailTexCoordPerMeter;
	return texture(albedoDetailSamplers[0], texCoord.xy);
}

vec4 sampleAttributeDetailTextures(vec3 normal, vec2 normalUv, vec3 albedo)
{
	vec3 detailTexCoordPerMeter = wrappedNoiseCoord * 5000.0;

	// Sample and blend attribute albedos
	vec4 attrWeights;

	vec2 albedoTexCoord = texCoord.xy * overallAlbedoMapUvScale + overallAlbedoMapUvOffset;
	ivec4 attrIndices = bilinearFetchIndicesFromAlbedo(albedo, attrWeights);

	// Do standard bilinear filtering
	vec4 attributeColor = vec4(0);
	
#define OVERLAY_BLEND
#ifdef OVERLAY_BLEND
	float noise = texture(albedoDetailSamplers[2], detailTexCoordPerMeter.xy*0.01).r;
	
	vec3 albedoDetailTexCoord = detailAlbedoUvScale[0] * detailTexCoordPerMeter;
	attributeColor = sampleDetailAlbedoMultiScale(0, normal, albedoDetailTexCoord, normalUv) * detailAlbedoBrightnessScale[0];
	
	float dirtSaturation = 1.5*saturation(albedo);
	attributeColor = mix(attributeColor, vec4(luminance(attributeColor.rgb)), max(0.0, 1-dirtSaturation));
	
	vec3 mask = attrWeights.xyz;
	vec3 falloff = vec3(0.3, 0.3, 1);
	mask = blendErosion(mask, vec3(noise), falloff);
	
	vec4 underlayAttributeColor = sampleDetailAlbedoMultiScale(1, normal, albedoDetailTexCoord, normalUv) * detailAlbedoBrightnessScale[1];
	attributeColor = mix(attributeColor, underlayAttributeColor, mask.g);
	
	underlayAttributeColor = sampleDetailAlbedoMultiScale(2, normal, albedoDetailTexCoord, normalUv) * detailAlbedoBrightnessScale[2];
	attributeColor = mix(attributeColor, underlayAttributeColor, mask.b);
	
#else
	for (int i = 0; i < 4; ++i)
	{
		int index = attrIndices[i];
		vec3 albedoDetailTexCoord = detailAlbedoUvScale[index] * detailTexCoordPerMeter;
		attributeColor += sampleDetailAlbedo(index, normal, albedoDetailTexCoord, normalUv) * attrWeights[i];
	}
#endif
	return attributeColor;
}

#endif

mat3 toTbnMatrix(vec3 normal)
{
	vec3 tangent = normalize(cross(-normal, vec3(0,1,0)));
	vec3 bitangent = cross(tangent, -normal);
	return transpose(mat3(tangent, bitangent, -normal));
}

vec3 getNormalWithBasis(vec3 localNormal, vec3 basisNormal)
{
	mat3 tbn = toTbnMatrix(basisNormal);
	return localNormal * tbn;
}

void main()
{
#ifdef CAST_SHADOWS
	return;
#endif

	vec3 positionRelCamera = position_worldSpace - cameraPosition;
	float fragmentViewDistance = length(positionRelCamera);
	
	vec2 normalUv = texCoord.xy * heightMapUvScale + heightMapUvOffset;
	
#ifdef ENABLE_OCEAN
	float landMask = texture(landMaskSampler, normalUv).a;
	
	// Discard and early out if terrain covered by ocean mesh
	if (landMask < 0.5)
	{
		if (fragmentViewDistance < oceanMeshFadeoutStartDistance && position_worldSpace.z > 0)
		{
			discard;
			return;
		}
	}
	
	const bool hasWater = (landMask < 0.999);
	const bool hasLand = (landMask > 0.0001);
#else
	const bool hasWater = false;
	const bool hasLand = true;
#endif

	vec3 viewDirection = -positionRelCamera / fragmentViewDistance;
	
	// Calculate normal
	vec3 normal = normalize(position_worldSpace - planetCenter);
	
	// Apply normal map
	if (hasLand)
	{
		vec3 localTerrainNormal = getTerrainNormalFromRG(texture(normalSampler, normalUv));
		localTerrainNormal = vec3(localTerrainNormal.z, localTerrainNormal.x, -localTerrainNormal.y);
		
		// TODO: can optimize by skipping basis adjustment if camera is close to earth's surface, since basis will be close to -z.
		normal = getNormalWithBasis(localTerrainNormal, normal);
	}

	// Calculate albedo
	vec3 waterAlbedo = deepScatterColor;
	vec3 landAlbedo;
	if (hasLand)
	{
		landAlbedo = texture(overallAlbedoSampler, texCoord.xy * overallAlbedoMapUvScale + overallAlbedoMapUvOffset).rgb;
		
	//#define FBM_DETAIL
	#ifdef FBM_DETAIL
		float fbmSignal = fbm(wrappedNoiseCoord.xy*10);
		
		landAlbedo.rgb = srgbToLinear(landAlbedo.rgb) * (1.0 + 0.4 * vec3(fbmSignal));
		landAlbedo.rgb = linearToSrgb(landAlbedo.rgb);
	#endif
		
	#if defined(DETAIL_MAPPING_TECHNIQUE_UNIFORM)
		landAlbedo += (dot(vec3(0.33333), sampleDetailTexture().rgb) - vec3(0.5))*0.6;
	#elif defined(DETAIL_MAPPING_TECHNIQUE_ALBEDO_DERIVED)
		vec3 attributeColor = sampleAttributeDetailTextures(normal, normalUv, landAlbedo).rgb;
		
		float albedoBlend = clamp(fragmentViewDistance / 8000, 0.0, 1.0);
		
		landAlbedo = mix(attributeColor * (vec3(0.5) + landAlbedo), landAlbedo, albedoBlend);
		//landAlbedo = mix(attributeColor, landAlbedo, albedoBlend);
	#endif
	}
	else
	{
		landAlbedo = vec3(0);
	}

	vec3 albedo = mix(waterAlbedo, landAlbedo, landMask);

	// Calculate lighting
#ifdef ENABLE_SHADOWS
	float lightVisibility = sampleShadowsAtWorldPosition(position_worldSpace, dot(normal, lightDirection), fragmentViewDistance);
#else
	float lightVisibility = 1.0;
#endif

	// Apply cloud shadows
	AtmosphericScattering scattering = scatteringPerVertex;
	{
		vec3 positionRelPlanet = position_worldSpace - planetCenter;
		vec3 cameraPositionRelPlanet = cameraPosition - planetCenter;
		scattering = applyCloudOcclusion(scattering, positionRelPlanet, cameraPositionRelPlanet, lightDirection, cloudSampler);
	}

	vec3 visibleSunIrradiance = scattering.sunIrradiance * lightVisibility;
	
	vec3 specularReflectance = vec3(0);

	if (hasWater)
	{
		vec3 waterSpecular = oceanSpecularColor * calcBlinnPhongSpecular(lightDirection, viewDirection, normal, oceanShininess) * visibleSunIrradiance;
		specularReflectance += (1 - landMask) * waterSpecular;
	}

	vec3 totalReflectance = albedo * (
			calcLambertDirectionalLight(lightDirection, normal) * visibleSunIrradiance + 	
			calcLambertSkyLight(lightDirection, normal) * scattering.skyIrradiance +
			ambientLightColor
		)
		+ specularReflectance;

	if (false && hasWater) // FIXME: currently disabled because it makes the high-res terrain tiles not match the low-res planet tiles
	{
		vec3 upDir = vec3(0,0,-1);
		float fresnel = calcSchlickFresnel(dot(viewDirection, upDir));
		totalReflectance = mix(totalReflectance, scattering.skyIrradiance, fresnel);
	}

#ifdef ENABLE_ATMOSPHERE
	color.rgb = totalReflectance * scattering.transmittance + scattering.skyRadianceToPoint;
#else
	color.rgb = totalReflectance;
#endif

	color.rgb = saturate(color.rgb); // Saturate to avoid fireflies from specular

//#define DEBUG_EDGES
#ifdef DEBUG_EDGES
	if(any(lessThan(texCoord.xy, vec2(0.02))))
	{
		color.rgb = vec3(1,0,0);
	}
#endif
	gl_FragDepth = logarithmicZ_fragmentShader(logZ);
	
	color.a = 1;
}