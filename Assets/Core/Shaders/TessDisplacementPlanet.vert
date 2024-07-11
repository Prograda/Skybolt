/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#version 400 core
#include "Planet.h"

in vec4 osg_Vertex;
in vec4 osg_MultiTexCoord0;
out vec3 vsTexCoord;
out vec3 vsUpDir;

uniform sampler2D heightSampler;
uniform mat4 modelMatrix;
uniform float heightScale;
uniform float heightOffset;
uniform vec2 heightMapUvScale;
uniform vec2 heightMapUvOffset;

void main()
{
	vec4 position = modelMatrix * osg_Vertex;
	vsUpDir = normalize(position.xyz - planetCenter);
	
	float radius = length(vec2(position.x, position.y));
	
	gl_Position.w = texture(heightSampler, osg_MultiTexCoord0.xy * heightMapUvScale + heightMapUvOffset).r * heightScale + heightOffset;
	gl_Position.xyz = position.xyz + vsUpDir * gl_Position.w;
	
	vsTexCoord.xy = osg_MultiTexCoord0.xy;
}