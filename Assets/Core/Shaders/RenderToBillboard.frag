/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#version 330 core

in vec3 texCoord;
in vec3 normal;

uniform sampler2D albedoSampler;

void main()
{
	vec4 color = texture(albedoSampler, texCoord.xy);
	if (color.a < 0.5)
		discard;
	
	gl_FragData[0] = vec4(color.rgb, 1);
	gl_FragData[1] = vec4(0.5 + 0.5 * normalize(normal), 1);
}