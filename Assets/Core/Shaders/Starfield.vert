/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#version 420 core

in vec4 osg_Vertex;
out float luminance;
uniform mat4 osg_ModelViewProjectionMatrix;
uniform mat4 viewProjectionMatrix;
uniform vec2 rcpWindowSizeInPixels;
uniform samplerBuffer paramsSampler;
uniform float brightness;

float scale = 2;
float luminanceReference = 5.0; // visually pleasing value, not realistic
float lowestVisibleMagnitude = 0;

void main()
{
	vec4 data = texelFetch(paramsSampler, gl_InstanceID);
	gl_Position = osg_ModelViewProjectionMatrix * vec4(data.xyz*10000, 1);
	
	gl_Position.xy += osg_Vertex.xy * gl_Position.w * scale * rcpWindowSizeInPixels;
	
	// Convert from magnitude to luminance based on: https://en.wikipedia.org/wiki/Absolute_magnitude
	float magnitude = data.w;
	luminance = pow(10, (magnitude-lowestVisibleMagnitude)/-2.5);
	luminance = brightness * luminance * luminanceReference;
}
