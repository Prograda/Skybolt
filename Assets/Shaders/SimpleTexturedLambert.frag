/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#version 440 core

#pragma import_defines ( ENABLE_ATMOSPHERE )
#pragma import_defines ( ENABLE_DEPTH_OFFSET )

#include "DepthPrecision.h"
#include "Brdfs/BlinnPhong.h"
#include "Brdfs/Lambert.h"

in vec3 texCoord;
in vec3 normalWS;
in float logZ;
in vec3 sunIrradiance;
in vec3 positionRelCamera;

#ifdef ENABLE_ATMOSPHERE
	in vec3 skyIrradiance;
	in vec3 transmittance;
	in vec3 skyRadianceToPoint;
#endif

out vec4 color;

uniform sampler2D albedoSampler;
uniform vec3 lightDirection;
uniform vec3 ambientLightColor;

#ifdef ENABLE_DEPTH_OFFSET
	uniform float depthOffset;
#endif

void main()
{
	color = texture(albedoSampler, texCoord.xy);
	
	color.rgb *= calcLambertDirectionalLight(lightDirection, normalWS) * sunIrradiance
#ifdef ENABLE_ATMOSPHERE
		+ calcLambertAmbientLight(normalWS, sunIrradiance, skyIrradiance)
#endif
		+ ambientLightColor;

#ifdef ENABLE_SPECULAR
	vec3 viewDirection = -normalize(positionRelCamera);
	color.rgb += 0.25 * calcBlinnPhongSpecular(lightDirection, viewDirection, normalWS, 20) * sunIrradiance;
#endif

#ifdef ENABLE_ATMOSPHERE
	color.rgb = color.rgb * transmittance + skyRadianceToPoint;
#endif
	gl_FragDepth = logarithmicZ_fragmentShader(logZ);

#ifdef ENABLE_DEPTH_OFFSET
	gl_FragDepth += depthOffset;
#endif
}
