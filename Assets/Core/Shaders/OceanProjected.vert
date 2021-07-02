/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#version 420 core
#include "DepthPrecision.h"
#include "AtmosphericScattering.h"
#include "Ocean.h"
#include "Planet.h"

in vec4 osg_Vertex;
out vec3 positionRelCameraWS;
out vec3 positionWS;
out float logZ;
out vec2 wrappedNoiseCoord;
out vec3 sunIrradiance;
out vec3 skyIrradiance;
out vec3 transmittance;
out vec3 skyRadianceToPoint;

uniform sampler2D heightSamplers[NUM_OCEAN_CASCADES];
uniform mat4 viewProjectionMatrix;
uniform vec3 cameraPosition;
uniform vec3 lightDirection;
uniform vec3 topLeftDir;
uniform vec3 topRightDir;
uniform vec3 bottomLeftDir;
uniform vec3 bottomRightDir;
uniform float waterHeight;
uniform vec2 heightMapTexCoordScales[NUM_OCEAN_CASCADES];

const float infinity = 1e10;

// @param normal does not need to be normalized going in
vec3 intersectPlane(in vec3 origin, in vec3 dir, float waterHeight)
{
  const vec3 normal = vec3(0,0,-1);
  float t = (waterHeight - dot(normal, origin)) / dot(normal, dir);
  if( t > 0.0 )
    return origin + dir * t;
  else
    return vec3(origin.x, origin.y, -waterHeight) + vec3(dir.x, dir.y, 0) * infinity;
}

const float screenCoordScale = 1.1; // TODO: tweak and optimize

void main()
{
	vec2 scaledVertexPos = osg_Vertex.xy * vec2(screenCoordScale); // +/- 0.5 at screen edges
	vec2 screenCoord = scaledVertexPos + vec2(0.5); // 0 and 1 at screen edges
	vec3 topDir = mix(topLeftDir, topRightDir, screenCoord.x);
	vec3 bottomDir = mix(bottomLeftDir, bottomRightDir, screenCoord.x);
	vec3 vertCameraWorldDir = mix(topDir, bottomDir, screenCoord.y);

	positionWS = intersectPlane(cameraPosition, vertCameraWorldDir, waterHeight);
	
	wrappedNoiseCoord = calcWrappedNoiseCoord(positionWS).xy;
	positionWS.z += planetSurfaceDrop(length(vec2(positionWS.x, positionWS.y)));
	
	float lod = length(positionWS) / 200; // TODO: should be based on texture-space size of projected quad onto ocean
	
	vec3 offset = vec3(0);
	for (int i = 0; i < NUM_OCEAN_CASCADES; ++i)
	{
		vec2 texCoord = wrappedNoiseCoord * heightMapTexCoordScales[i];
		offset += textureLod(heightSamplers[i], texCoord, lod).xyz;
	}

#define PIN_TO_SCREEN_EDGES
#ifdef PIN_TO_SCREEN_EDGES
	vec2 edgeFeather = (abs(scaledVertexPos) - vec2(0.5)) / vec2(0.05); // 0 at edge and 1 at maximum distance beyond edge
	float edgeFeatherScalar = max(0.0, max(edgeFeather.x, edgeFeather.y));
	offset.xy *= mix(1.0, 0.0, edgeFeatherScalar);
#endif

	positionWS.xyz += offset * vec3(1,1,-1);// * max(0.0, 1.0 - length(positionWS) / 5000);
	
	positionRelCameraWS = positionWS - cameraPosition;
	
	gl_Position = viewProjectionMatrix * vec4(positionWS, 1);
	gl_Position.z = logarithmicZ_vertexShader(gl_Position.z, gl_Position.w, logZ);
	
	// Atmospheric scattering
	vec3 positionRelPlanet = positionWS.xyz - planetCenter;
	vec3 cameraPositionRelPlanet = cameraPosition - planetCenter;
	skyRadianceToPoint = GetSkyRadianceToPoint(cameraPositionRelPlanet, positionRelPlanet, 0, lightDirection, transmittance);
	sunIrradiance = GetSunAndSkyIrradiance(positionRelPlanet, lightDirection, skyIrradiance);
}