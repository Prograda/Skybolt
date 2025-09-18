/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <optional>

namespace skybolt {
namespace vis {

struct WaveSpectrumWindow
{
	// Window is expressed as wavelengths rather than frequency to make parameters easier to reason about.

	std::optional<float> cutoffWavelengthShortest; //!< If set, cuts off wavelengths shorter than this value
	std::optional<float> cutoffWavelengthLongest; //!< If set, cuts off wavelengths longer than this value
};

inline float calcNyquistWavelength(float textureWorldSize, int textureSizePixels)
{
	return 2 * textureWorldSize / textureSizePixels;
}

} // namespace vis
} // namespace skybolt
