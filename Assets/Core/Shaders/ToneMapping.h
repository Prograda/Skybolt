/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef TONE_MAPPING_H
#define TONE_MAPPING_H

#include "ColorTemperature.h"
#include "Util/Srgb.h"

#pragma import_defines ( CONVERT_OUTPUT_TO_SRGB )

vec3 getOutputAsSrgb(vec3 color)
{
#ifdef CONVERT_OUTPUT_TO_SRGB
	// This is more accurate than pow(color, vec3(1.0 / 2.2))
	return linearToSrgb(color);
#else
	return color;
#endif
}

// Tone Mapping curve by Hajime Uchimura used in Gran Torismo.
// See https://bruop.github.io/tonemapping
vec3 toneMap_Uchimura(vec3 x, vec3 P, float a, vec3 m, vec3 l, vec3 c, vec3 b) {
    // Uchimura 2017, "HDR theory and practice"
    // Math: https://www.desmos.com/calculator/gslcdxvipg
    // Source: https://www.slideshare.net/nikuque/hdr-theory-and-practicce-jp
    vec3 l0 = ((P - m) * l) / a;
    vec3 L0 = m - m / a;
    vec3 L1 = m + (1.0 - m) / a;
    vec3 S0 = m + l0;
    vec3 S1 = m + a * l0;
    vec3 C2 = (a * P) / (P - S1);
    vec3 CP = -C2 / P;

    vec3 w0 = 1.0 - smoothstep(vec3(0.0), m, x);
    vec3 w2 = step(m + l0, x);
    vec3 w1 = 1.0 - w0 - w2;

    vec3 T = m * pow(x / m, c) + b;
    vec3 S = P - (P - S1) * exp(CP * (x - S0));
    vec3 L = m + a * (x - m);

    return T * w0 + L * w1 + S * w2;
}

vec3 toneMap_Uchimura(vec3 x) {
    const vec3 P = vec3(1.0);  // max display brightness
    const float a = 1.0;  // contrast
    const vec3 m = vec3(0.22); // linear section start
    const vec3 l = vec3(0.4);  // linear section length
    const vec3 c = vec3(1.33); // black
    const vec3 b = vec3(0.0);  // pedestal
    return toneMap_Uchimura(x, P, a, m, l, c, b);
}

// Tone Mapping curve by John Hable used in Uncharted 2.
// See https://www.gdcvault.com/play/1012351/Uncharted-2-HDR
vec3 toneMap_uncharted2(vec3 x)
{
    float A = 0.15f;
    float B = 0.50f;
    float C = 0.10f;
    float D = 0.20f;
    float E = 0.02f;
    float F = 0.30f;
    return ((x*(A*x+C*B)+D*E)/(x*(A*x+B)+D*F))-E/F;
}

vec3 toneMapWithWhitePointAdjustment(vec3 v, vec3 whitePoint)
{
    vec3 curr = toneMap_Uchimura(v);
    vec3 white_scale = vec3(1.0f) / toneMap_Uchimura(whitePoint);
    return curr * white_scale;
}

float brightnessMultiplier = 4.0;

vec3 toneMap(vec3 color)
{
#ifdef APPLY_COLOR_TEMPERATURE
    vec3 whitePoint = vec3(colorTemperatureToRGB(6500));
#else
	vec3 whitePoint = vec3(1);
#endif
	
	color *= brightnessMultiplier;
	color = toneMapWithWhitePointAdjustment(color, whitePoint);

	return getOutputAsSrgb(color);
}

#endif // TONE_MAPPING_H
