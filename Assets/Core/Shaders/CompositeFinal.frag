/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#version 330 core

#pragma import_defines(ENABLE_CLOUDS)

#include "ToneMapping.h"

in vec3 texCoord;
uniform sampler2D mainColorTexture;
uniform sampler2D mainDepthTexture;
uniform sampler2D cloudsColorTexture;

out vec4 color;

void main()
{
	color = texture(mainColorTexture, texCoord.xy);
	
#ifdef ENABLE_CLOUDS
	// Composite clouds
	vec4 cloudsColor = texture(cloudsColorTexture, texCoord.xy);
	
	// Square the color value stored in the texture to undo the sqrt that was applied when the texture was written to.
	// This gets us back into linear color space.
	cloudsColor.rgba *= cloudsColor.rgba;
	color = color * (1.0 - cloudsColor.a) + cloudsColor; // composite with pre-multiplied alpha
#endif
	
	color.rgb = toneMap(color.rgb);
	gl_FragDepth = texture(mainDepthTexture, texCoord.xy).r;
}
