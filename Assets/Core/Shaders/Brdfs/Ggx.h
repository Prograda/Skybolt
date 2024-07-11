/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef GGX_H
#define GGX_H

#include "GlobalDefines.h"

float G1V(float dotNV, float k)
{
	// Schlick-smith approximation of Smith.
	// Smith's original function is:
	// 1.0 / sqrt((-dotNV * k * k + dotNV) * dotNV + k * k);
	// For comparison, see https://www.desmos.com/calculator/wtp8lnjutx
	// Note: this does not incldue the dotNL*dotNV multiplication
	// from the original Schlick model because the dotNL is handled
	// by the caller, and the dotNV cancels out in rendering.
	return 1.0f/(dotNV*(1.0f-k)+k);
}

float calcGgxD(float dotNH, float alpha)
{
	float alphaSqr = alpha * alpha;
	float denom = dotNH * dotNH *(alphaSqr-1.0f) + 1.0f;
	return alphaSqr /( M_PI * denom * denom);
}

// Exact Schlick-smith V calculation.
// This is the visibility term, i.e the masking term (G) divided by the projected area.
// This does not incldue the division by 4 for microfact model normalization,
// which must be handled separately.
float calcGgxV(float dotNL, float dotNV, float alpha)
{
	float k = alpha * 0.5f; // half the alpha to better approximate smith function
	return G1V(dotNL,k) * G1V(dotNV,k);
}

// GGX visibility term approximation by John Hable
// http://filmicworlds.com/blog/optimizing-ggx-shaders-with-dotlh/
// This does not incldue the division by 4 for microfact model normalization,
// which must be handled separately.
float calcGgxVApprox(float dotLH, float alpha)
{
	float k = alpha * 0.5f; // half the alpha to better approximate smith function
	float r = G1V(dotLH,k);
	return r*r;
}

#endif // define GGX_H