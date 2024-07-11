/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#version 330 core

in vec3 positionModelSpace;
in vec3 positionRelCameraWS;
in vec3 offsetWS;
in vec2 uvCoord;

out vec4 color;

uniform sampler2D albedoSampler;
uniform vec3 cameraPosition;
uniform vec3 lightDirection;

const float ambient = 0.5;
const float phaseG = 0.7;

// Returns average of 1
float calcPhase(float cosTheta, float g)
{
	return max(0.0, 1 - g * sign(cosTheta) * pow(abs(cosTheta), 10));
}

float sqr(float a)
{
	return a*a;
}

void main()
{
	vec3 normal = offsetWS;
	float offsetLength = length(normal);
	if (offsetLength > 1.0)
		normal /= offsetLength;
	
	vec3 viewDirection = normalize(positionRelCameraWS);
	
	float normalRelView = sqrt(max(0.0, 1.0 - dot(normal,normal)));
	normal = normal - viewDirection * normalRelView;

	float LDotN = dot(normal, lightDirection);
	float luminance = mix(ambient, 1.0, sqr(LDotN * 0.5 + 0.5));
	
	float alpha = texture(albedoSampler, uvCoord).a;
	float cosTheta = dot(-viewDirection, lightDirection);
	float phase = calcPhase(cosTheta, phaseG);
	luminance *= mix(phase, 1.0, 0.7*alpha);
	
	float ambientLight = clamp(0.2 - lightDirection.z, 0.01, 1);
	luminance *= ambientLight;
	
	color = vec4(vec3(luminance), alpha);
	color.a = (color.a - 0.05) / (1.0-0.05); // TODO: remove this line after fixing alpha channel
	
	if (color.a < 0.005)
		discard;
}