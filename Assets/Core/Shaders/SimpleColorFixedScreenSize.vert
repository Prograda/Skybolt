/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#version 330

in vec4 osg_Vertex;
in vec4 osg_MultiTexCoord0;
in vec4 osg_Color;

out vec4 vsColor;
out vec4 texCoord;

uniform mat4 osg_ModelViewProjectionMatrix;
uniform vec2 screenSizePixels;

void main(void)
{
	gl_Position = osg_ModelViewProjectionMatrix * vec4(0,0,0,1);
	gl_Position.xy += osg_Vertex.xy * (vec2(200.0f) / screenSizePixels) * gl_Position.w;
	texCoord = osg_MultiTexCoord0;
	vsColor = osg_Color;
}
