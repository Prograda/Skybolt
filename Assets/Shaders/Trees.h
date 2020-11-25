/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef TREES_H
#define TREES_H

vec3 randomizeColor(vec3 sourceColor, float unitRandom)
{
	return sourceColor * vec3(mix(0.7, 1.6, unitRandom));
}

#endif // TREES_H