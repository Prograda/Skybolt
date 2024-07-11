/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#version 330 core

in vec3 texCoord;

out vec4 color;

uniform sampler2D albedoSampler;

float brightness = 0.5; // TODO: adjust

void main()
{
	color = texture(albedoSampler, texCoord.xy) * brightness;
}