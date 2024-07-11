/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#version 440 core

#include "DepthClamp.h"
in vec4 osg_Vertex;
in vec4 osg_MultiTexCoord0;

out vec3 texCoord;

uniform mat4 osg_ModelViewProjectionMatrix;

void main()
{
	gl_Position = osg_ModelViewProjectionMatrix * osg_Vertex;
	gl_Position.z = calcFarDepthClampedZ(gl_Position);

	texCoord = osg_MultiTexCoord0.xyz;
}
