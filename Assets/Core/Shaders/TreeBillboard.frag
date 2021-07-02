/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#version 330 core
#include "DepthPrecision.h"
#include "Trees.h"
#include "Brdfs/Lambert.h"

in vec2 texCoord;
in float perTreeUnitRandom;
in vec3 normal;
in float logZ;
in vec3 irradiance;
in vec3 transmittance;
in vec3 skyRadianceToPoint;
out vec4 color;

uniform sampler2D albedoSampler;
uniform vec3 lightDirection;
uniform vec3 ambientLightColor;

void main()
{
	color = texture(albedoSampler, texCoord.xy);

	if (color.a < 0.5)
	discard;
	
	color.rgb = randomizeColor(color.rgb, perTreeUnitRandom);

	color.rgb *= calcLambertSkyLight(lightDirection, normal) * irradiance + ambientLightColor;
		
	color.rgb = color.rgb * transmittance + skyRadianceToPoint;
	
	gl_FragDepth = logarithmicZ_fragmentShader(logZ);
}