/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#version 330 core

in vec3 texCoord;

out vec4 color;

uniform sampler2D displacementSampler;
uniform sampler2D prevOutputSampler;
uniform vec2 texelSizeInTextureSpace;
uniform vec2 texelSizeInWorldSpace;
uniform float lambda;
uniform float foamMaskSubtractionAmount; //!< This amount will be each frame

void main()
{
	vec2 halfTexelSize = 0.5 * texelSizeInTextureSpace;

	vec3 x0 = texture(displacementSampler, texCoord.xy + vec2(-halfTexelSize.x, 0)).xyz;
	vec3 x1 = texture(displacementSampler, texCoord.xy + vec2( halfTexelSize.x, 0)).xyz;
	vec3 y0 = texture(displacementSampler, texCoord.xy + vec2(0, -halfTexelSize.y)).xyz;
	vec3 y1 = texture(displacementSampler, texCoord.xy + vec2(0,  halfTexelSize.y)).xyz;

	// Reference: https://arm-software.github.io/opengl-es-sdk-for-android/ocean_f_f_t.html
	vec2 lambdaOnH = lambda / (2.0 * texelSizeInWorldSpace);
	float Jxx = 1 + lambdaOnH.x * (x1.x - x0.x);
	float Jyy = 1 + lambdaOnH.y * (y1.y - y0.y);
	float Jxy = lambdaOnH.y * (x1.y - x0.y);
	float Jyx = lambdaOnH.x * (y1.x - y0.x);

	float J = Jxx * Jyy - Jxy * Jyx;
	color = vec4(0.5-J);
	
	// Max with slightly darkened output from previous frame to make foam slowly fade out over time
	color = max(color, texture(prevOutputSampler, texCoord.xy).rrrr - vec4(foamMaskSubtractionAmount));
}
