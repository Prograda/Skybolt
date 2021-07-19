/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#version 440 core

#pragma import_defines ( ENABLE_ATMOSPHERE )
#pragma import_defines ( ENABLE_NORMAL_MAP )

#include "AtmosphericScatteringWithClouds.h"
#include "CloudShadows.h"
#include "DepthPrecision.h"
#include "Shadows/Shadows.h"

in vec4 osg_Vertex;
in vec4 osg_Normal;
in vec4 osg_MultiTexCoord0;
in vec4 osg_MultiTexCoord1;

out vec3 texCoord;
out vec3 normalWS;
out vec3 tangentWS;
out vec3 positionRelCamera;
out float logZ;
out vec3 sunIrradiance;
out vec3 shadowTexCoord;
out AtmosphericScattering scattering;

uniform mat4 osg_ModelViewProjectionMatrix;
uniform mat4 modelMatrix;
uniform vec3 cameraPosition;
uniform vec3 lightDirection;

uniform sampler2D cloudSampler;

void main()
{
	gl_Position = osg_ModelViewProjectionMatrix * osg_Vertex;
	gl_Position.z = logarithmicZ_vertexShader(gl_Position.z, gl_Position.w, logZ);
	
	texCoord = osg_MultiTexCoord0.xyz;
	normalWS = mat3(modelMatrix) * osg_Normal.xyz;
	
#ifdef ENABLE_NORMAL_MAP
	tangentWS = mat3(modelMatrix) * osg_MultiTexCoord1.xyz;
#endif
	
	vec4 positionWS = modelMatrix * osg_Vertex;
	
#ifdef ENABLE_ATMOSPHERE
	// Atmospheric scattering
	vec3 positionRelPlanet = positionWS.xyz - planetCenter;
	vec3 cameraPositionRelPlanet = cameraPosition - planetCenter;
	scattering = calcAtmosphericScattering(cameraPositionRelPlanet, positionRelPlanet, lightDirection, cloudSampler);
#else
	sunIrradiance = GetSunIrradianceInSpace();
#endif
	shadowTexCoord = (shadowProjectionMatrix0 * positionWS).xyz;
}