/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef COLOR_TEMPERATURE_H
#define COLOR_TEMPERATURE_H

// Based on implementation by Benjamin 'BeRo' Rosseaux.
// https://www.shadertoy.com/view/4sc3D7
// Valid from 1000 to 40000 K (and additionally 0 for pure full white)
vec3 colorTemperatureToRGB(const in float temperature)
{
	// Values from: http://blenderartists.org/forum/showthread.php?270332-OSL-Goodness&p=2268693&viewfull=1#post2268693
	mat3 m = (temperature <= 6500.0)
		? mat3(vec3(0.0, -2902.1955373783176, -8257.7997278925690),
			vec3(0.0, 1669.5803561666639, 2575.2827530017594),
			vec3(1.0, 1.3302673723350029, 1.8993753891711275)) : 
		mat3(vec3(1745.0425298314172, 1216.6168361476490, -8257.7997278925690),
			vec3(-2666.3474220535695, -2173.1012343082230, 2575.2827530017594),
			vec3(0.55995389139931482, 0.70381203140554553, 1.8993753891711275));
										  
	return mix(clamp(vec3(m[0] / (vec3(clamp(temperature, 1000.0, 40000.0)) + m[1]) + m[2]), vec3(0.0), vec3(1.0)), vec3(1.0), smoothstep(1000.0, 0.0, temperature));
}

#endif // COLOR_TEMPERATURE_H
