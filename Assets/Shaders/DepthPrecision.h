/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

float far = 5e7; // TODO
float C = 1; // TODO: adjust to avoid non-linear poly intersection artifacts near the camera
	
float logarithmicZ(float w) 
{
	w = max(1e-8f, w);
	float logZ = log(C*w + 1) / log(C*far + 1);
	return (2*logZ - 1) * w;
}

float z_logarithmic(float w)
{
	return (C*w + 1);
}

float fragDepth_logarithmic(float z)
{
	return log(z) / log(C*far + 1);
}