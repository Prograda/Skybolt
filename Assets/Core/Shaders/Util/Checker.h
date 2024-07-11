/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef CHECKER_H
#define CHECKER_H

// From https://www.geeks3d.com/hacklab/20190225/demo-checkerboard-in-glsl/
float checker(vec2 uv, float repeats) 
{
	float cx = floor(repeats * uv.x);
	float cy = floor(repeats * uv.y); 
	float result = mod(cx + cy, 2.0);
	return sign(result);
}

#endif // CHECKER_H