/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#version 440 core
#include "AtmosphericScatteringWithClouds.h"
#include "DepthPrecision.h"

#pragma import_defines ( CAST_SHADOWS )
#pragma import_defines ( ENABLE_ATMOSPHERE )

in vec4 osg_Vertex;
in vec4 osg_MultiTexCoord0;
out vec2 texCoord;
out vec2 normalViewSpaceXY;
out vec3 emissionColor;
out float alpha;
out float logZ;
out vec3 positionRelCamera;
out vec3 lightDirectionViewSpace;
out AtmosphericScattering scattering;

uniform mat4 osg_ModelViewProjectionMatrix;
uniform mat4 viewMatrix;
uniform vec3 cameraPosition;
uniform vec3 cameraUpDirection;
uniform vec3 cameraRightDirection;
uniform vec3 lightDirection;

uniform sampler2D cloudSampler;

vec2 rotate(vec2 v, float a)
{
	float s = sin(a);
	float c = cos(a);
	mat2 m = mat2(c, -s, s, c);
	return m * v;
}

vec3 colorAtTemperatureDegreesCelcius(float temperature)
{
	// This function is not physically based, but looks plausible
	// Scale so objects are black around 500 deg and white around 2000 deg
    float t = max(0.0, (temperature - 500.0) * 0.0013);
	
    return 5 * vec3(
		pow(t, 1.5),
		pow(t, 2.5) * 0.4,
		pow(t, 3.5) * 0.15);
}

void main()
{
	float x = ((gl_VertexID + 1) % 4) > 1 ? 1.0f : 0.0f;
	float y = (gl_VertexID % 4) / 2;
	
	texCoord = vec2(x, y);
	
	vec2 offset = texCoord * 2.0 - vec2(1.0);
	
	// Rotation
	float rotationAngle = osg_MultiTexCoord0.z;
	offset = rotate(offset, rotationAngle);

	normalViewSpaceXY = offset * 0.8;
	
	// Scale
	offset *= osg_MultiTexCoord0.xx;
	
	vec4 pos = osg_Vertex;
	pos.xyz += offset.x * cameraRightDirection + offset.y * cameraUpDirection;
	
	emissionColor = colorAtTemperatureDegreesCelcius(osg_MultiTexCoord0.w);
	alpha = osg_MultiTexCoord0.y;

	gl_Position = osg_ModelViewProjectionMatrix * pos;
	
#ifdef CAST_SHADOWS
	return;
#endif
	gl_Position.z = logarithmicZ_vertexShader(gl_Position.z, gl_Position.w, logZ);
	

	vec3 positionWS = osg_Vertex.xyz; // assume particles are in world coordinates
	positionRelCamera = positionWS.xyz - cameraPosition;

	lightDirectionViewSpace = mat3(viewMatrix) * lightDirection;
		
#ifdef ENABLE_ATMOSPHERE
	// Atmospheric scattering
	vec3 positionRelPlanet = positionWS.xyz - planetCenter;
	vec3 cameraPositionRelPlanet = cameraPosition - planetCenter;
	scattering = calcAtmosphericScattering(cameraPositionRelPlanet, positionRelPlanet, lightDirection, cloudSampler);
#else
	scattering.sunIrradiance = GetSunIrradianceInSpace();
	scattering.skyIrradiance = vec3(0);
#endif
	
}
