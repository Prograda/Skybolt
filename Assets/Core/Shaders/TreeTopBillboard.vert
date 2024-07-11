/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#version 420 core
#pragma import_defines ( ENABLE_CLOUDS )
#pragma import_defines ( GPU_PLACEMENT )

#include "AtmosphericScattering.h"
#include "CloudShadows.h"
#include "DepthPrecision.h"
#include "GlobalDefines.h"
#include "Noise/FastRandom.h"

#ifdef GPU_PLACEMENT
#include "TreeBillboards.h"
#endif

in vec4 osg_Vertex;

out vec2 texCoord;
out float perTreeUnitRandom;
out vec3 normal;
out float logZ;
out vec3 sunIrradiance;
out vec3 skyIrradiance;
out vec3 transmittance;
out vec3 skyRadianceToPoint;

uniform mat4 osg_ModelViewProjectionMatrix;
uniform mat4 modelMatrix;
uniform float maxVisibilityRange;
uniform vec3 cameraPosition;
uniform vec3 lightDirection;

#define QUARTER_PI 0.78539816339

vec3 calcXDir(int i) {return vec3(cos(i*QUARTER_PI), sin(i*QUARTER_PI), 0);}
vec3 calcYDir(int i) {return vec3(-sin(i*QUARTER_PI), cos(i*QUARTER_PI), 0);}

const vec3 xDirs[8] = vec3[8](calcXDir(0), calcXDir(1), calcXDir(2), calcXDir(3), calcXDir(4), calcXDir(5), calcXDir(6), calcXDir(7));
const vec3 yDirs[8] = vec3[8](calcYDir(0), calcYDir(1), calcYDir(2), calcYDir(3), calcYDir(4), calcYDir(5), calcYDir(6), calcYDir(7));

uniform samplerBuffer treeParamsSampler;
uniform sampler2D cloudSampler;

void main()
{
	vec4 pos = osg_Vertex;
	
#ifdef GPU_PLACEMENT // place trees on GPU
	int attributeId = getTerrainAttributeId(pos.xy);
	if (attributeId != 9)
	{
		gl_Position = vec4(0,0,0,1);
		return;
	}

	pos.xyz = getWorldBillboardPositionFromVertexPosition(pos.xy);
	
	if (pos.z > -3)
	{
		gl_Position = vec4(0,0,0,1);
		return;
	}
#endif
	
	vec4 worldPos = modelMatrix * pos;
	
	vec3 posRelCamera = vec3(worldPos.xyz - cameraPosition);
	
	float x = ((gl_VertexID + 1) % 4) > 1 ? 1.0f : 0.0f;
	float y = (gl_VertexID % 4) / 2;

	int id = (gl_VertexID / 4) % textureSize(treeParamsSampler);;
	vec4 data = texelFetch(treeParamsSampler, id);
	float type = data.r;
	float height = data.g;
	vec2 billboardSize = vec2(height * 0.5f); // up facing imposters are half the texture dimensions of side facing texture height
	
	int yawIndex = int(floor(7.999f * data.b * M_RCP_2PI));
	vec3 xDir = xDirs[yawIndex];
	vec3 yDir = yDirs[yawIndex];
	
	//float visibility = float(length(posRelCamera) < maxVisibilityRange);
	float visibility = clamp((maxVisibilityRange*1.1 - length(posRelCamera))*0.001, 0.0f, 1.0f);
	
	vec3 posOffset = xDir * (x - 0.5) * billboardSize.x * visibility + yDir * (y - 0.5) * billboardSize.y * visibility;
	pos.xyz += posOffset;
	pos.z -= height * 0.25f;
	
	gl_Position = osg_ModelViewProjectionMatrix * pos;
	gl_Position.z = logarithmicZ_vertexShader(gl_Position.z, gl_Position.w, logZ);
	
	texCoord = vec2(x, y);
	texCoord.y = (texCoord.y + (3-type)) / 4.0f;
	
	perTreeUnitRandom = randomFast1d(float(id));

	float horizontalNormalScale = 2.5; // scale normal so that it reaches 1 at the edge of the tree. This accounts for wasted texture space around the tree texture.
	normal.xyz = (xDir * (x - 0.5) + yDir * (y - 0.5)) * horizontalNormalScale;
	normal.z = -sqrt(max(0.0, (1.0 - dot(normal,normal))));

	// Atmospheric scattering
	vec3 positionRelPlanet = worldPos.xyz - planetCenter;
	vec3 cameraPositionRelPlanet = cameraPosition - planetCenter;
	skyRadianceToPoint = GetSkyRadianceToPoint(cameraPositionRelPlanet, positionRelPlanet, 0, lightDirection, transmittance);
	sunIrradiance = GetSunAndSkyIrradiance(positionRelPlanet, lightDirection, skyIrradiance);
	
#ifdef ENABLE_CLOUDS
	sunIrradiance *= sampleCloudShadowMaskAtPositionRelPlanet(cloudSampler, positionRelPlanet, lightDirection);
#endif
	float occlusion = y;
	sunIrradiance *= occlusion;
	skyIrradiance *= occlusion;
}