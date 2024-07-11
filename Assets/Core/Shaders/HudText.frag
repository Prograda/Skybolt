/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#version 330 core

in vec4 vsColor;
in vec4 texCoord;
uniform sampler2D glyphTexture;

out vec4 color;

void main()
{
	vec4 albedo = texture(glyphTexture, texCoord.xy);
	color = vec4(vsColor.xyz, albedo.a);
}
