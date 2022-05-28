/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#version 440 core
#include "AtmosphericScatteringWithClouds.h"
#include "DepthPrecision.h"

#pragma import_defines ( CAST_SHADOWS )

in vec4 osg_Vertex;
in vec3 normal;
in vec4 osg_MultiTexCoord0;
out vec2 texCoord;
out float alpha;
out float logZ;

uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;

void main()
{
	vec3 normalViewSpace = mat3(modelViewMatrix) * normal;
	
	vec4 pos = modelViewMatrix * osg_Vertex;
	pos.xyz += normalize(cross(normalViewSpace, vec3(0,0,1))) * osg_MultiTexCoord0.z;
	gl_Position = projectionMatrix * pos;
#ifdef CAST_SHADOWS
	return;
#endif
	gl_Position.z = logarithmicZ_vertexShader(gl_Position.z, gl_Position.w, logZ);

	texCoord = osg_MultiTexCoord0.xy;
	alpha = osg_MultiTexCoord0.w;
}
