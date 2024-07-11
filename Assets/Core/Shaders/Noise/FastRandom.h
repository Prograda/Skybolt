/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef FAST_RANDOM_H
#define FAST_RANDOM_H

// Returns value between 0 and 1
float randomFast1d(float n) { return fract(sin(n) * 43758.5453123); }

vec3 randomFast3d(vec3 n) { return fract(sin(n) * 43758.5453123); }

#endif // FAST_RANDOM_H