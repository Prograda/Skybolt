/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#version 420 core
#include "DepthPrecision.h"
#include "AtmosphericScatteringWithClouds.h"
#include "CloudShadows.h"
#include "Ocean.h"
#include "Planet.h"

in vec4 osg_Vertex;
out vec3 positionRelCameraWS;
out vec3 positionWS;
out float logZ;
out vec2 wrappedNoiseCoord;
out AtmosphericScattering scattering;

uniform sampler2D heightSamplers[NUM_OCEAN_CASCADES];
uniform sampler2D cloudSampler;
uniform mat4 viewProjectionMatrix;
uniform vec3 cameraPosition;
uniform vec3 lightDirection;
uniform mat4 modelMatrix;
uniform sampler2D heightSampler;
uniform vec2 heightMapTexCoordScale;
uniform float displacementHeight;
uniform vec2 heightMapTexCoordScales[NUM_OCEAN_CASCADES];

void main()
{
	positionWS = vec3(modelMatrix * osg_Vertex).xyz;
	
	wrappedNoiseCoord = calcWrappedNoiseCoord(positionWS).xy;
	// Surface drop is disabled because it drops lakes too far compared with terrain. TODO: investigate.
	//positionWS.z += planetSurfaceDrop(length(vec2(positionWS.x, positionWS.y)));
	
	float lod = length(positionWS) / 200; // TODO: should be based on texture-space size of projected quad onto ocean
	
	vec3 offset = vec3(0);
	for (int i = 0; i < NUM_OCEAN_CASCADES; ++i)
	{
		vec2 texCoord = wrappedNoiseCoord * heightMapTexCoordScales[i];
		offset += textureLod(heightSamplers[i], texCoord, lod).xyz;
	}
	
	positionWS.xyz += offset * vec3(1,1,-1);// * max(0.0, 1.0 - length(positionWS) / 5000);
	
	positionRelCameraWS = positionWS - cameraPosition;
	
	gl_Position = viewProjectionMatrix * vec4(positionWS, 1);
	gl_Position.z = logarithmicZ_vertexShader(gl_Position.z, gl_Position.w, logZ);
	
	// Atmospheric scattering
	vec3 positionRelPlanet = positionWS.xyz - planetCenter;
	vec3 cameraPositionRelPlanet = cameraPosition - planetCenter;
	scattering = calcAtmosphericScattering(cameraPositionRelPlanet, positionRelPlanet, lightDirection, cloudSampler);
}