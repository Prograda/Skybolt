/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#version 440 core
#include "DepthPrecision.h"

in vec3 texCoord;
in float logZ;

out vec4 color;

uniform sampler2D albedoSampler;

void main()
{
	color = texture(albedoSampler, texCoord.xy);
	gl_FragDepth = logarithmicZ_fragmentShader(logZ);
}