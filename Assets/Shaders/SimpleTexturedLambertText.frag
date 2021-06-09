/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#version 440 core

#pragma import_defines ( ENABLE_DEPTH_OFFSET )

#include "DepthPrecision.h"
#include "Brdfs/Lambert.h"

in vec3 texCoord;
in vec3 normalWS;
in float logZ;
in vec3 sunIrradiance;
in vec3 skyIrradiance;
in vec3 transmittance;
in vec3 skyRadianceToPoint;

out vec4 color;

uniform sampler2D albedoSampler;
uniform vec3 lightDirection;
uniform vec3 ambientLightColor;

#ifdef ENABLE_DEPTH_OFFSET
	uniform float depthOffset;
#endif

void main()
{
	color = vec4(1, 1, 1, texture(albedoSampler, texCoord.xy).a);
	
	if (color.a < 0.5)
		discard;
	
	color.rgb *= calcLambertDirectionalLight(lightDirection, normalWS) * sunIrradiance
		+ calcLambertSkyLight(lightDirection, normalWS) * skyIrradiance
		+ ambientLightColor;

	color.rgb = color.rgb * transmittance + skyRadianceToPoint;
	
	gl_FragDepth = logarithmicZ_fragmentShader(logZ);

#ifdef ENABLE_DEPTH_OFFSET
	gl_FragDepth += depthOffset;
#endif
}
