/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#version 440 core
#include "AtmosphericScatteringWithClouds.h"
#include "DepthPrecision.h"
#include "EnvironmentMap.h"
#include "Brdfs/BlinnPhong.h"
#include "Util/Saturate.h"

in vec3 positionRelCamera;
in vec3 normalWS;
in AtmosphericScattering scattering;
in float logZ;
in vec4 vsColor;
out vec4 color;

uniform vec3 ambientLightColor;
uniform vec3 lightDirection;
uniform sampler2D environmentSampler;

void main()
{
	vec3 viewDirection = -normalize(positionRelCamera);
	vec3 reflection = calcBlinnPhongSpecular(lightDirection, viewDirection, normalWS, 100) * scattering.sunIrradiance;
	
	vec3 reflectionDir = reflect(-viewDirection, normalWS);
	
	vec2 environmentUv = getSphericalEnvironmentMapTexCoord(reflectionDir);
	reflection += textureLod(environmentSampler, environmentUv, 3).rgb;
	
	float reflectionMultiplier = 0.1;
	color.rgb = saturate((ambientLightColor + reflection) * reflectionMultiplier); // saturate to avoid fireflies from specular
	color.a = 0.7;
	gl_FragDepth = logarithmicZ_fragmentShader(logZ);
}
