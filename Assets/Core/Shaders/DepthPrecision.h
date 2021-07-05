/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Enables more accurate Log Z calculation by calculating it in the fragment shader
// instead of the vertex shader. This is slower, but avoids z fighting artifacts on
// on sparsely tesselated geometry.
#pragma import_defines(ACCURATE_LOG_Z)

uniform float farClipDistance;
float C = 0.5; // Adjust to avoid non-linear poly intersection artifacts near the camera

// Logarithmic Z buffer trick from https://outerra.blogspot.com/2012/11/maximizing-depth-buffer-range-and.html

//! Calculates NDC depth value from a w value, for a logarithmic depth buffer
float calcLogZNdc(float clipSpaceW)
{
	return log(C*clipSpaceW + 1) / log(C*farClipDistance + 1);
}

// This calculates logarithmic Z in the vertex shader and avoids fragment shader computation,
// with the drawback of less precision close to the camera.
// @return a new clipSpaceZ to be assigned to gl_Position.z
// @param zForFrag is the parameter to be passed into logarithmicZ_fragmentShader
float logarithmicZ_vertexShader(float clipSpaceZ, float clipSpaceW, out float zForFrag) 
{
#ifdef ACCURATE_LOG_Z
	zForFrag = C*clipSpaceW + 1;
	return clipSpaceZ;
#else
	zForFrag = calcLogZNdc(clipSpaceW);
	return (2*zForFrag - 1) * clipSpaceW;
#endif
}

// Calculates gl_FragDepth value,
// where z is the zForFrag calculated from logarithmicZ_vertexShader in the vertex shader.
float logarithmicZ_fragmentShader(float z)
{
#ifdef ACCURATE_LOG_Z
	return log(z) / log(C*farClipDistance + 1);
#else
	return z;
#endif
}

// TODO:
// As per outerra blog post, we can use conservative depth in fragment shader to increase performance
// #extension GL_ARB_conservative_depth : enable
// layout(depth_less) out float gl_FragDepth;