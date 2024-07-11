/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#version 440 core
#include "DepthPrecision.h"

in float logZ;
in vec4 vsColor;
out vec4 color;

void main()
{
	color = vsColor;
	gl_FragDepth = logarithmicZ_fragmentShader(logZ);
}
