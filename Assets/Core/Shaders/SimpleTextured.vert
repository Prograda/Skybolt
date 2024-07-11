/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#version 440 core

#pragma import_defines ( CAST_SHADOWS )
#pragma import_defines ( ENABLE_ATMOSPHERE )
#pragma import_defines ( ENABLE_NORMAL_MAP )
#pragma import_defines ( FLIP_V )

#include "AtmosphericScatteringWithClouds.h"
#include "CloudShadows.h"
#include "DepthPrecision.h"

in vec4 osg_Vertex;
in vec4 osg_Normal;
in vec4 osg_MultiTexCoord0;
in vec4 osg_MultiTexCoord1;

out vec3 texCoord;
out vec3 normalWS;
out vec3 tangentWS;
out vec3 positionRelCamera;
out float logZ;
out vec3 shadowTexCoord;
out AtmosphericScattering scattering;

uniform mat4 osg_ModelViewProjectionMatrix;
uniform mat4 modelMatrix;
uniform vec3 cameraPosition;
uniform vec3 lightDirection;
uniform mat4 shadowProjectionMatrix0;

uniform sampler2D cloudSampler;

void main()
{
	gl_Position = osg_ModelViewProjectionMatrix * osg_Vertex;
	
#ifdef CAST_SHADOWS
	return;
#endif
	
	gl_Position.z = logarithmicZ_vertexShader(gl_Position.z, gl_Position.w, logZ);
	
	texCoord = osg_MultiTexCoord0.xyz;
	
#ifdef FLIP_V
	texCoord.y = 1.0 - texCoord.y;
#endif
	
	normalWS = mat3(modelMatrix) * osg_Normal.xyz;
	
#ifdef ENABLE_NORMAL_MAP
	tangentWS = mat3(modelMatrix) * osg_MultiTexCoord1.xyz;
#endif
	
	vec4 positionWS = modelMatrix * osg_Vertex;
	positionRelCamera = positionWS.xyz - cameraPosition;
	
#ifdef ENABLE_ATMOSPHERE
	// Atmospheric scattering
	vec3 positionRelPlanet = positionWS.xyz - planetCenter;
	vec3 cameraPositionRelPlanet = cameraPosition - planetCenter;
	scattering = calcAtmosphericScattering(cameraPositionRelPlanet, positionRelPlanet, lightDirection, cloudSampler);
#else
	scattering.sunIrradiance = GetSunIrradianceInSpace();
	scattering.skyIrradiance = vec3(0);
#endif
	shadowTexCoord = (shadowProjectionMatrix0 * positionWS).xyz;
}
