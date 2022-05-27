/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#version 440 core
#include "AtmosphericScatteringWithClouds.h"
#include "DepthPrecision.h"
#include "Brdfs/HenyeyGreenstein.h"

#pragma import_defines ( CAST_SHADOWS )

in vec2 texCoord;
in vec2 normalViewSpaceXY;
in vec3 emissionColor;
in float alpha;
in float logZ;
in vec3 positionRelCamera;
in vec3 lightDirectionViewSpace;
in AtmosphericScattering scattering;
out vec4 color;

uniform sampler2D albedoSampler;
uniform vec3 lightDirection;
uniform vec3 ambientLightColor;
uniform vec3 cameraUpDirection;

void main()
{
	color = texture(albedoSampler, texCoord.xy);
	color.a *= alpha;
	
#ifdef CAST_SHADOWS
	if (color.a < 0.5)
	{
		discard;
	}
	return;
#endif
	vec3 normalViewSpace = vec3(normalViewSpaceXY, sqrt(1.0 - dot(normalViewSpaceXY, normalViewSpaceXY)));
	float dotNL = dot(lightDirectionViewSpace, normalViewSpace);
	float halfLambert = max(0.0, (dotNL * 0.8 + 0.2)) / M_PI;

	float dotVL = dot(lightDirection, normalize(positionRelCamera));
	float hg = henyeyGreenstein(dotVL, 0.2);
	
	float softness = 0.5; // blend between lambert and HG BRDFs
	color.rgb = mix(halfLambert, hg, softness) * (scattering.sunIrradiance + scattering.skyIrradiance)
		+ emissionColor
		+ ambientLightColor;

	gl_FragDepth = logarithmicZ_fragmentShader(logZ);
}
