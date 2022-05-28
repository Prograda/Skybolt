/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#version 440 core
#include "DepthPrecision.h"
#include "GlobalDefines.h"
#include "Util/Srgb.h"

#pragma import_defines ( CAST_SHADOWS )

in vec2 texCoord;
in float alpha;
in float logZ;
out vec4 color;

uniform sampler2D colorSampler;

void main()
{
	color = texture(colorSampler, texCoord.xy);
	color.rgb /= M_PI;
	
// Convert alpha to linear since it was authored in sRGB (i.e in photoshop)
#define CONVERT_ALPHA_TO_LINEAR
#ifdef CONVERT_ALPHA_TO_LINEAR
	color.a = srgbToLinear(color.a);
#endif

	color.a *= alpha;
	
#ifdef CAST_SHADOWS
	if (color.a < 0.5)
	{
		discard;
	}
	return;
#endif	
	gl_FragDepth = logarithmicZ_fragmentShader(logZ);
}
