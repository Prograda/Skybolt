/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#version 330 core

in vec3 texCoord;
uniform sampler2D colorTexture;
uniform sampler2D depthTexture;

out vec4 color;

void main()
{
	color = texture(colorTexture, texCoord.xy);
	gl_FragDepth = texture(depthTexture, texCoord.xy).r*0.99; // add small bias to avoid z fighting

	// Square the color value stored in the texture to undo the sqrt that was applied when the texture was written to.
	// This gets us back into linear color space.
	color.rgba *= color.rgba;
}
