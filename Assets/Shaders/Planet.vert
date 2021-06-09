/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#version 330 core
#pragma import_defines ( ENABLE_ATMOSPHERE )

#include "AtmosphericScattering.h"
#include "DepthPrecision.h"
#include "Planet.h"
#include "Brdfs/Lambert.h"

in vec4 osg_Vertex;
in vec4 osg_MultiTexCoord0;

out vec2 geoTexCoord;
out vec2 albedoTexCoord;
out vec3 normal;
out float logZ;
out vec3 sunIrradiance;
out vec3 skyIrradiance;
out vec3 transmittance;
out vec3 skyRadianceToPoint;
out vec3 positionRelCamera;

uniform mat4 viewProjectionMatrix;
uniform mat4 modelMatrix;
uniform vec3 cameraPosition;
uniform vec3 lightDirection;
uniform vec2 albedoImageScale;
uniform vec2 albedoImageOffset;

void main()
{
	geoTexCoord = osg_MultiTexCoord0.xy;
	albedoTexCoord = osg_MultiTexCoord0.xy * albedoImageScale + albedoImageOffset;
	vec3 positionWorldSpace = vec4(modelMatrix * osg_Vertex).xyz;
	
	positionRelCamera = positionWorldSpace - cameraPosition;

	vec3 positionRelPlanet = positionWorldSpace - planetCenter;
	normal = normalize(positionRelPlanet);

	gl_Position = viewProjectionMatrix * vec4(positionWorldSpace, 1);
	gl_Position.z = logarithmicZ_vertexShader(gl_Position.z, gl_Position.w, logZ);
	
	// Calculate lighting
#ifdef ENABLE_ATMOSPHERE
	vec3 cameraPositionRelPlanet = cameraPosition - planetCenter;
	skyRadianceToPoint = GetSkyRadianceToPoint(cameraPositionRelPlanet, positionRelPlanet, 0, lightDirection, transmittance);
	sunIrradiance = GetSunAndSkyIrradiance(positionRelPlanet, lightDirection, skyIrradiance);
#else
	skyRadianceToPoint = vec3(0);
	transmittance = vec3(1);
	sunIrradiance = GetSunIrradianceInSpace();
	skyIrradiance = vec3(0);
#endif
}
