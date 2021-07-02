/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#version 330 core

#include "ToneMapping.h"

in vec3 texCoord;
uniform sampler2D inputSampler;

out vec4 color;

void main()
{
	color = texture(inputSampler, texCoord.xy);
	color.rgb = toneMap(color.rgb);
}
