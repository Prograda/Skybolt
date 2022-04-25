/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#version 330 core
#include "DepthPrecision.h"
#include "OrenNayar.h"

#define PI 3.14159265359
#define TWO_PI 6.28318530718
#define THREE_PI 9.42477796077

in vec3 texCoord;

out vec4 color;

uniform sampler2D albedoSampler;
uniform float moonPhase;

float lighting(vec2 uv, float phase)
{
    float alpha = 1.0;

	vec3 N;
	N.xy = 2 * (uv - vec2(0.5));
	N.z = sqrt(1 - dot(N.xy, N.xy));
	
	float angle = PI + phase * TWO_PI;
	
	vec3 L = vec3(-sin(angle),0,cos(angle));

	return orenNayar(L, vec3(0, 0, 1), N, 0.4);
}

float bounceLightFromEarth = 0.005;

void main()
{
	color = max(bounceLightFromEarth, lighting(texCoord.xy, moonPhase)) * texture(albedoSampler, texCoord.xy);
}