/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#version 330 core
#include "DepthPrecision.h"

in vec4 osg_Vertex;
in int gl_VertexID;
in vec4 osg_MultiTexCoord0;
in vec4 osg_MultiTexCoord1;

out vec3 positionModelSpace;
out vec3 positionRelCameraWS;
out vec3 offsetWS;
out vec2 uvCoord;

uniform vec3 cameraPosition;
uniform mat4 osg_ModelViewProjectionMatrix;

vec2 getUvCoordsInAtlas(vec2 uvCoord, int textureId)
{
	int x = textureId % 4;
	int y = textureId / 4;
	return (vec2(float(x), float(y)) + uvCoord) / 4.0;
}

void main()
{
	vec4 positionWS = osg_Vertex;
	positionRelCameraWS = positionWS.xyz - cameraPosition;
	
	vec3 upDir = vec3(0,0,-1);
	vec3 cameraDir = normalize(positionRelCameraWS);
	vec3 sideDir = normalize(cross(cameraDir, upDir));
	upDir = cross(cameraDir, sideDir);
	
	vec2 offset = osg_MultiTexCoord0.xy * 2 - vec2(1);
	vec3 dimensions = osg_MultiTexCoord1.xyz;
	offsetWS = sideDir * offset.x + upDir * offset.y;
		
	positionWS.xyz += offsetWS * dimensions;
	
	gl_Position = osg_ModelViewProjectionMatrix * positionWS;
	
	int textureId = (gl_VertexID / 4) % 4 + 12;
	uvCoord = getUvCoordsInAtlas(osg_MultiTexCoord0.xy, textureId);
	
	gl_Position.z = logarithmicZ(gl_Position.w);
}