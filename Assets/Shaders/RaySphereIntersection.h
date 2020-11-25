/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef RAY_SPHERE_INTERSECTION_H
#define RAY_SPHERE_INTERSECTION_H

// r0: ray origin
// rd: normalized ray direction
// s0: sphere center
// sr: sphere radius
// Returns distance from r0 to first intersection with sphere, or -1.0 if no intersection.
float raySphereFirstIntersection(vec3 r0, vec3 rd, vec3 s0, float sr)
{
    vec3 s0_r0 = r0 - s0;
    float b = 2.0 * dot(rd, s0_r0);
    float c = dot(s0_r0, s0_r0) - (sr * sr);
	
	float det = b*b - 4.0*c;
    if (det < 0.0)
	{
        return -1.0;
    }
    return (-b - sqrt(det)) * 0.5;
}

// r0: ray origin
// rd: normalized ray direction
// s0: sphere center
// sr: sphere radius
// Returns distance from r0 to second intersection with sphere, or -1.0 if no intersection.
float raySphereSecondIntersection(vec3 r0, vec3 rd, vec3 s0, float sr)
{
    vec3 s0_r0 = r0 - s0;
    float b = 2.0 * dot(rd, s0_r0);
    float c = dot(s0_r0, s0_r0) - (sr * sr);

	float det = b*b - 4.0*c;
    if (det < 0.0)
	{
        return -1.0;
    }
    return (-b + sqrt(det)) * 0.5;
}

// r0: ray origin
// rd: normalized ray direction
// s0: sphere center
// sr: sphere radius
// Returns distances to intersections t0 and t1. Sets t0 to -1 if there was no intersection.
void raySphereIntersections(vec3 r0, vec3 rd, vec3 s0, float sr, out float t0, out float t1)
{
    vec3 s0_r0 = r0 - s0;
    float b = 2.0 * dot(rd, s0_r0);
    float c = dot(s0_r0, s0_r0) - (sr * sr);

	float det = b*b - 4.0*c;
    if (det < 0.0)
	{
        t0 = -1.0;
    }
	float sqrtDet = sqrt(det);
    t0 = (-b - sqrtDet) * 0.5;
	t1 = (-b + sqrtDet) * 0.5;
}

// r0: ray origin
// rd: normalized ray direction
// s0: sphere center
// sr: sphere radius
// Returns distance from r0 to first intersection with sphere, or -1.0 if no intersection.
float raySphereFirstPositiveIntersection(vec3 r0, vec3 rd, vec3 s0, float sr)
{
    vec3 s0_r0 = r0 - s0;
    float b = 2.0 * dot(rd, s0_r0);
    float c = dot(s0_r0, s0_r0) - (sr * sr);
	
	float det = b*b - 4.0*c;
    if (det < 0.0)
	{
        return -1.0;
    }
	float twoT0 = -b - sqrt(det);
	if (twoT0 > 0.0)
	{
		return twoT0 * 0.5;
	}
	return -b + sqrt(det);
}

#endif // RAY_SPHERE_INTERSECTION_H