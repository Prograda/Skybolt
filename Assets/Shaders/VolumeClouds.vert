/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#version 330 core

#include "GlobalUniforms.h"

in vec4 osg_Vertex;

out vec3 vertCameraWorldDir;
out float cameraAltitude;

uniform vec3 cameraPosition;
uniform vec3 topLeftDir;
uniform vec3 topRightDir;
uniform vec3 bottomLeftDir;
uniform vec3 bottomRightDir;

void main()
{
	gl_Position = osg_Vertex * vec4(2) - vec4(1);
	
	vec2 screenCoord = osg_Vertex.xy;
	vec3 topDir = mix(topLeftDir, topRightDir, screenCoord.x);
	vec3 bottomDir = mix(bottomLeftDir, bottomRightDir, screenCoord.x);
	vertCameraWorldDir = mix(topDir, bottomDir, screenCoord.y);

	cameraAltitude = length(cameraPosition - planetCenter) - innerRadius;
}