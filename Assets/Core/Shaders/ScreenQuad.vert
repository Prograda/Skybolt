/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#version 330 core

#pragma import_defines(USE_WORLD_SPACE_POSITION) // Position quat at screen-projected world-space position
#pragma import_defines(USE_ASPECT_RATIO_CORRECTION) // Adjusts x coordinate by aspect ratio

in vec4 osg_Vertex;
in vec4 osg_MultiTexCoord0;

out vec3 texCoord;

uniform mat4 osg_ModelViewProjectionMatrix;
uniform float rcpViewAspectRatio;

void main()
{
	vec4 vertexPosition = osg_Vertex;
#ifdef USE_ASPECT_RATIO_CORRECTION
	vertexPosition.xy *= vec2(rcpViewAspectRatio, 1.0);
#endif
#ifdef USE_WORLD_SPACE_POSITION
	gl_Position = osg_ModelViewProjectionMatrix * vec4(0,0,0,1);
	gl_Position.xy += vertexPosition.xy * gl_Position.w;
#else
	gl_Position = vertexPosition * vec4(2) - vec4(1);
#endif
	texCoord = osg_MultiTexCoord0.xyz;
}