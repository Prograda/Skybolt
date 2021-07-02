/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#version 330 core

in vec4 osg_Vertex;
in vec4 osg_Normal;
in vec4 osg_MultiTexCoord0;

out vec3 texCoord;
out vec3 normal;

uniform mat4 osg_ModelViewProjectionMatrix;
uniform mat4 osg_ModelViewMatrix;

void main()
{
	gl_Position = osg_ModelViewProjectionMatrix * osg_Vertex;
	texCoord = osg_MultiTexCoord0.xyz;
	normal = mat3(osg_ModelViewMatrix) * osg_Normal.xyz;
}