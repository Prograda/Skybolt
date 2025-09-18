/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <algorithm>
#include <cmath>
#include <stdint.h>

#include <SkyboltCommon/Math/MathUtility.h>

namespace skybolt {
namespace vis {

inline uint32_t hashCoords(int n, int m, uint32_t seed = 0)
{
    uint32_t x = static_cast<uint32_t>(n);
    uint32_t y = static_cast<uint32_t>(m);
    uint32_t h = x * 374761393u + y * 668265263u + seed * 0x27d4eb2d;
    h = (h ^ (h >> 13)) * 1274126177u;
    return h;
}

inline float hashToUniform(uint32_t h)
{
    return (h & 0x00FFFFFF) / float(0x01000000); // 24-bit mantissa
}

//! @returns a value sampled from the standard normal distrubtion (a gaussian with mean=0 and standard deviation=1)
//! @param u1 is a random value in range [0, 1)
//! @param u2 is a random value in range [0, 1)
inline float uniformToStandardNormal(float u1, float u2)
{
    u1 = std::max(u1, 1e-7f); // Prevent log(0)
    float r = std::sqrt(-2.0f * std::log(u1));
    float theta = math::twoPiF() * u2;
    return r * std::cos(theta);
}

inline float deterministicNormalFast(int n, int m, uint32_t seed = 0)
{
    uint32_t h1 = hashCoords(n, m, seed);
    uint32_t h2 = hashCoords(n, m, seed + 47);
    float u1 = hashToUniform(h1);
	float u2 = hashToUniform(h2);
    return uniformToStandardNormal(u1, u2);
}

} // namespace skybolt
} // namespace vis