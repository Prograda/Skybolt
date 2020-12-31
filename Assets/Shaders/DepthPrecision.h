/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

uniform float farClipDistance;
float C = 1.0; // Adjust to avoid non-linear poly intersection artifacts near the camera

// Logarithmic Z buffer trick from https://outerra.blogspot.com/2012/11/maximizing-depth-buffer-range-and.html
float logarithmicZ(float w) 
{
	w = max(1e-8f, w);
	float logZ = log(C*w + 1) / log(C*farClipDistance + 1);
	return (2*logZ - 1) * w;
}

float z_logarithmic(float w)
{
	return (C*w + 1);
}

// Calculates log-corrected gl_FragColor value,
// where z is calculated from z_logarithmic in the vertex shader.
float fragDepth_logarithmic(float z)
{
	return log(z) / log(C*farClipDistance + 1);
}

// TODO:
// As per outerra blog post, we might be able to use conservative depth in fragment shader to increase performance
// #extension GL_ARB_conservative_depth : enable
// layout(depth_less) out float gl_FragDepth;