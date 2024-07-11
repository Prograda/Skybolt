/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#version 330 core
#include "AtmosphericScattering.h"
#include "DepthClamp.h"

// Based on GPU Gems 2 Chapter 16. Accurate Atmospheric Scattering.

in vec4 osg_Vertex;

out vec3 scattering;
out vec3 singleScattering;
out vec3 fragPosRelCamera;

uniform mat4 modelMatrix;
uniform mat4 osg_ModelViewProjectionMatrix;
uniform vec3 lightDirection;
uniform vec3 cameraPosition;

void main()
{
	gl_Position = osg_ModelViewProjectionMatrix * osg_Vertex;
	gl_Position.z = calcFarDepthClampedZ(gl_Position);

	// Get the ray from the camera to the vertex, and its length (which is the far point of the ray passing through the atmosphere)
	vec3 cameraPositionRelPlanetCenter = cameraPosition - planetCenter;

	vec3 vertPosWorld = osg_Vertex.xyz + planetCenter;
	
	fragPosRelCamera = vertPosWorld - cameraPosition;
	
	vec3 rayDir = normalize(fragPosRelCamera);
	
	vec3 transmittance;
	scattering = GetSkyRadianceScattering(cameraPositionRelPlanetCenter, rayDir, 0, lightDirection, transmittance, singleScattering);
}