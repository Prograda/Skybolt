/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef DEPTH_CLAMP_H
#define DEPTH_CLAMP_H

// Clamp z to keep vertex within far-plane of frustum, which effectivly disables far plane clipping.
// This is an alternative to glEnable(GL_DEPTH_CLAMP) in cases where GL_DEPTH_CLAMP cannot be used or is unsupported.
float calcFarDepthClampedZ(vec4 positionClipspace)
{
	return min(gl_Position.z, gl_Position.w*0.99);
}

#endif // DEPTH_CLAMP_H