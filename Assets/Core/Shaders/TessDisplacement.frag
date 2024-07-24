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

in vec3 texCoord;
in vec3 position_worldSpace;
in vec3 wrappedNoiseCoord;
in float cameraDistance;
in float elevation;
in AtmosphericScattering scattering;
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

uniform float forestAlbedoTransitionRange;
const float oneOnForestAlbedoBlendDistance = 1.0f / 1000.0f;
const float barrenCoverageAmounts[11] = float[11](1,0,0,0, 0,0,0,0, 0,0,0);
const float forestCoverageAmounts[11] = float[11](0,0,0,0, 0,0,0,0, 0,1,0);
const float sandDepositableAmounts[11] = float[11](1,0,0,0, 0,1,1,1, 1,1,0);
const float snowCoverageAmounts[11] = float[11](0,0,0,0, 0,0,0,0, 0,0,1);

const float detailAlbedoUvScale[11] = float[11](0.01,0.01,0.01, 0.001,0.001,0.01, 0.01,0.001,0.001, 0.001,0.01);

const float vegetationSlopeSinTheta = 0.7f;
const float oneOnVegetationSlopeBlend = 100.0;

float snowShininess = 16;


vec4 textureTriplanar(sampler2D sampler, vec3 position, vec3 normal)
{
	float tighten = 0.4679f; 
	vec3 weights = clamp(abs(normal) - tighten, 0, 1);
	
	float total = weights.x + weights.y + weights.z;
	weights /= total;
	
	vec4 cXY = texture(sampler, position.xy);
	vec4 cXZ = texture(sampler, position.xz);
	vec4 cYZ = texture(sampler, position.yz);	
	
	return weights.z * cXY + weights.y * cXZ + weights.x * cYZ; 
}

ivec4 bilinearFetchIndices(sampler2D tex, vec2 uv, out vec4 weights)
{// TODO: consider clamping at edges here
	ivec2 dims = textureSize(tex, 0);
	vec2 coordFloat = uv * vec2(dims) - 0.5f;
	ivec2 coordInt = ivec2(floor(coordFloat));
	vec2 weight = coordFloat - coordInt;
	coordInt = clamp(coordInt, ivec2(0), dims-ivec2(2));
	
	float c00 = texelFetch(tex, coordInt, 0).a;
	float c10 = texelFetch(tex, coordInt + ivec2(1, 0), 0).a;
	float c01 = texelFetch(tex, coordInt + ivec2(0, 1), 0).a;
	float c11 = texelFetch(tex, coordInt + ivec2(1, 1), 0).a;

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

float calcSlopeConvergence(vec2 uv)
{
	vec2 offset = 1.0 / textureSize(normalSampler, 0);
	vec2 x0 = getTerrainNormalFromRG(texture(normalSampler, uv + vec2(-offset.x, 0))).xz;
	vec2 x1 = getTerrainNormalFromRG(texture(normalSampler, uv + vec2( offset.x, 0))).xz;
	vec2 y0 = getTerrainNormalFromRG(texture(normalSampler, uv + vec2(0, -offset.y))).xz;
	vec2 y1 = getTerrainNormalFromRG(texture(normalSampler, uv + vec2(0,  offset.y))).xz;
	
	return (x0.x - x1.x) + (y0.y - y1.y);
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
vec4 sampleDetailTexture()
{
	vec3 detailTexCoordPerMeter = wrappedNoiseCoord * 10000.0; // repeats [0,1) per meter
	vec3 texCoord = detailAlbedoUvScale[0] * detailTexCoordPerMeter;
	return texture(albedoDetailSamplers[0], texCoord.xy);
}
#endif // DETAIL_SAMPLER_COUNT

#if DETAIL_SAMPLER_COUNT > 1
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

vec4 sampleAttributeDetailTextures(vec3 normal, vec2 normalUv, vec3 albedo)
{
	vec3 detailTexCoordPerMeter = wrappedNoiseCoord * 5000.0; // repeats [0,1) per meter
	vec2 attributeTexCoord = texCoord.xy * attributeMapUvScale + attributeMapUvOffset;

	// Sample and blend attribute albedos
	vec4 attrWeights;

	vec2 albedoTexCoord = texCoord.xy * overallAlbedoMapUvScale + overallAlbedoMapUvOffset;
	ivec4 attrIndices = bilinearFetchIndicesFromAlbedo(albedo, attrWeights);

//#define DETAIL_HEIGHTMAP_BLEND
#ifdef DETAIL_HEIGHTMAP_BLEND
	// Use the detail maps' alpha channel as a heightmap to do height-aware blending between materials.
	// Texels with a higher height will show on top, e.g grass should appear over dirt.
	vec4 attributeColors[4];
	float maxHeight = 0.0;
	for (int i = 0; i < 4; ++i)
	{
		int index = attrIndices[i];

		vec3 albedoDetailTexCoord = detailAlbedoUvScale[index] * detailTexCoordPerMeter;
		attributeColors[i] = sampleDetailAlbedo(index, normal, albedoDetailTexCoord, normalUv);
		attributeColors[i].a *= attrWeights[i];
		maxHeight = max(maxHeight, attributeColors[i].a);
	}
	
	maxHeight -= 0.1; // controls the amount of feather on the blend
	
	vec4 attributeColor = vec4(0);
	float totalWeight = 0;
	for (int i = 0; i < 4; ++i)
	{
		float weight = max(attrWeights[i] * (attributeColors[i].a - maxHeight), 0);
		attributeColor += attributeColors[i] * weight;
		totalWeight += weight;
	}
	attributeColor /= totalWeight;
	//attributeColor = vec4(maxHeight);
	
#else
	// Do standard bilinear filtering
	vec4 attributeColor = vec4(0);
	
#define OVERLAY_BLEND
#ifdef OVERLAY_BLEND
	float noise = texture(albedoDetailSamplers[2], detailTexCoordPerMeter.xy*0.01).r;
	
	vec3 albedoDetailTexCoord = detailAlbedoUvScale[0] * detailTexCoordPerMeter;
	attributeColor = sampleDetailAlbedoMultiScale(0, normal, albedoDetailTexCoord, normalUv);
	
	float dirtSaturation = 1.5*saturation(albedo);
	attributeColor = mix(attributeColor, vec4(luminance(attributeColor.rgb)), max(0.0, 1-dirtSaturation));
	
	vec3 mask = attrWeights.xyz;
	vec3 falloff = vec3(0.3, 0.3, 1);
	mask = blendErosion(mask, vec3(noise), falloff);
	
	vec4 underlayAttributeColor = sampleDetailAlbedoMultiScale(1, normal, albedoDetailTexCoord, normalUv);
	attributeColor = mix(attributeColor, underlayAttributeColor, mask.g);
	
	underlayAttributeColor = sampleDetailAlbedoMultiScale(2, normal, albedoDetailTexCoord, normalUv);
	attributeColor = mix(attributeColor, underlayAttributeColor, mask.b);
	
#else
	
	for (int i = 0; i < 4; ++i)
	{
		int index = attrIndices[i];
		vec3 albedoDetailTexCoord = detailAlbedoUvScale[index] * detailTexCoordPerMeter;
		attributeColor += sampleDetailAlbedo(index, normal, albedoDetailTexCoord, normalUv) * attrWeights[i];
	}
#endif //OVERLAY_BLEND
#endif
	return attributeColor;
}

#endif // DETAIL_SAMPLER_COUNT > 1

#ifdef EXPERIMENTAL_DETAIL_ALBEDO
vec4 sampleDetailAlbedoAdvanced(int i, vec3 normal, vec3 texCoord, vec2 normalUv)
{

	if (i == 0)	// barren
	{
		color = textureTriplanar(albedoDetailSamplers[1], texCoord, normal);
		
		float slope = 1 + normal.z;
		float slopeNoise = scaledFbm(wrappedNoiseCoord, 100) * 0.2;
		float slopeWeight = saturate(rerange(slope + slopeNoise, 0.15, 0.5));
		
		vec3 rockTextureNormal = normalize(vec3(normal.x, normal.y, 0));
		vec4 rockColor = textureTriplanar(albedoDetailSamplers[0], texCoord * 0.2, rockTextureNormal);
		color = blend(color, 1-slopeWeight, rockColor, slopeWeight);
		//color = mix(color, rockColor, slopeWeight);
		
		float deposition = calcSlopeConvergence(normalUv);
		float depositionNoise = scaledFbm(wrappedNoiseCoord, 200) * 0.2;
		float depositionWeight = saturate(rerange(deposition + depositionNoise, 0, 1));
		
		vec4 depositionColor = textureTriplanar(albedoDetailSamplers[2], texCoord, normal);
		color = blend(color, 1-depositionWeight, depositionColor, depositionWeight);
		//color = mix(color, depositionColor, depositionWeight);
		color.a = 1;
		
	}
	else if (i < 10)
	{
		color = texture(albedoDetailSamplers[i], texCoord.xy);
		color.r += 0.05 * scaledFbm(wrappedNoiseCoord, 10);
		color.g += 0.05 * scaledFbm(wrappedNoiseCoord, 80);
	}
	else
	{
		color = vec4(1); // snow
	}
	
	return color;
}
#endif

vec3 fbmNormal()
{
	vec3 period = vec3(10);
	vec3 p = wrappedNoiseCoord * period;
	
	float step = 0.1;
	float scale = 1 * step;
	float h00 = fbm(p, period);
	float h10 = fbm(p + vec3(step,0,0), period);
	float h01 = fbm(p + vec3(0,step,0), period);
	vec3 tangent = vec3(step,0,scale*(h10 - h00));
	vec3 bitangent = vec3(0,step,scale*(h01 - h00));
	return normalize(cross(bitangent, tangent));
}

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
	const bool isWater = (landMask < 0.5);
	
	// Discard and early out if terrain covered by ocean mesh
	if (isWater)
	{
		if (fragmentViewDistance < oceanMeshFadeoutStartDistance && position_worldSpace.z > 0)
		{
			discard;
			return;
		}
	}
#else
	const bool isWater = false;
#endif

	vec3 viewDirection = -positionRelCamera / fragmentViewDistance;
	
	// Calculate normal
	vec3 normal = normalize(position_worldSpace - planetCenter);
	if (!isWater)
	{
		vec3 localTerrainNormal = getTerrainNormalFromRG(texture(normalSampler, normalUv));
		localTerrainNormal = vec3(localTerrainNormal.z, localTerrainNormal.x, -localTerrainNormal.y);
		
		// TODO: can optimize by skipping basis adjustment if camera is close to earth's surface, since basis will be close to -z.
		normal = getNormalWithBasis(localTerrainNormal, normal);
		
#ifdef ADD_NORMAL_FBM_NOISE
		normal = getNormalWithBasis(fbmNormal(), normal);
#endif
	}

	// Calculate albedo
	vec3 albedo;
	if (isWater)
	{
		albedo = deepScatterColor;
	}
	else
	{
		albedo = texture(overallAlbedoSampler, texCoord.xy * overallAlbedoMapUvScale + overallAlbedoMapUvOffset).rgb;

	#if defined(DETAIL_MAPPING_TECHNIQUE_UNIFORM)
		albedo += (dot(vec3(0.33333), sampleDetailTexture().rgb) - vec3(0.5))*0.6;
	#elif defined(DETAIL_MAPPING_TECHNIQUE_ALBEDO_DERIVED)
		vec3 attributeColor = 0.9*sampleAttributeDetailTextures(normal, normalUv, albedo).rgb;
		float albedoBlend = clamp(fragmentViewDistance / 8000, 0.0, 1.0);
		
		albedo = mix(attributeColor * (vec3(0.5) + albedo), albedo, albedoBlend);
		//albedo = mix(attributeColor, albedo, albedoBlend);
	#endif
	}

	// Calculate lighting
#ifdef ENABLE_SHADOWS
	float lightVisibility = sampleShadowsAtWorldPosition(position_worldSpace, dot(normal, lightDirection), fragmentViewDistance);
#else
	float lightVisibility = 1.0;
#endif

	vec3 visibleSunIrradiance = scattering.sunIrradiance * lightVisibility;
	
	vec3 specularReflectance = vec3(0);

	if (isWater)
	{
		specularReflectance += oceanSpecularColor * calcBlinnPhongSpecular(lightDirection, viewDirection, normal, oceanShininess) * visibleSunIrradiance;
	}

	vec3 totalReflectance = albedo * (
			calcLambertDirectionalLight(lightDirection, normal) * visibleSunIrradiance + 	
			calcLambertSkyLight(lightDirection, normal) * scattering.skyIrradiance +
			ambientLightColor
		)
		+ specularReflectance;

	if (false && isWater) // FIXME: currently disabled because it makes the high-res terrain tiles not match the low-res planet tiles
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
	
	// Hack to ensure terrain is drawn under water plane
	if (isWater)
	{
		gl_FragDepth += 0.0002;
	}
	
	color.a = 1;
}