/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#version 330 core
#include "DepthPrecision.h"

in vec4 osg_Vertex;
in vec4 osg_Color;

out vec4 vsColor;
out float logZ;

uniform mat4 osg_ModelViewProjectionMatrix;

void main()
{
	gl_Position = osg_ModelViewProjectionMatrix * osg_Vertex;
	vsColor = osg_Color;

	gl_Position.z = logarithmicZ(gl_Position.w);
	logZ = z_logarithmic(gl_Position.w);
}
