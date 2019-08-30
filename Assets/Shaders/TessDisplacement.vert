/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#version 400 core

in vec4 osg_Vertex;
in vec4 osg_MultiTexCoord0;
out vec3 vsTexCoord;
out vec3 vsUpDir;

uniform sampler2D heightSampler;
uniform float heightScale;
uniform vec2 heightMapUvScale;
uniform vec2 heightMapUvOffset;

void main()
{
	vsUpDir = vec3(0,0,-1);

	gl_Position.w = (texture(heightSampler, osg_MultiTexCoord0.xy * heightMapUvScale + heightMapUvOffset).r - 0.5f) * heightScale;
	gl_Position.xyz = osg_Vertex.xyz + vsUpDir * gl_Position.w;
	
	vsTexCoord = osg_MultiTexCoord0.xyz;
}