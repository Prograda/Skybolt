/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltVis/SkyboltVisFwd.h"
#include "SkyboltVis/Image/Image.h"
#include <SkyboltCommon/Exception.h>

#include <glm/glm.hpp>
#include <string>

namespace skybolt {
namespace vis {

inline float srgbGamma() { return 2.2f; }
inline float rcpSrgbGamma() { return 1.0f / srgbGamma(); }

glm::vec4 applyGamma(const glm::vec4& c, float gamma);

inline glm::vec4 srgbToLinear(const glm::vec4& c)
{
	return applyGamma(c, srgbGamma());
}

inline glm::vec4 linearToSrgb(const glm::vec4& c)
{
	return applyGamma(c, rcpSrgbGamma());
}

//! Returns the SRGB-space average colour of an SRGB-space image
glm::vec4 averageSrgbColor(const Image& image, float alphaRejectionThreshold = 0.5);

//! @param coord is in pixel coordinates (not normalized)
glm::vec4 getColorBilinear(const Image& image, const glm::vec2& coord);

void normalize(Image& image);

void flipVertical(Image& image);

//! @return color of image at coord.
//! @param coord is normalized in range [0, 1]. Origin at top left.
glm::vec4 getColorAtNormCoord(const Image& image, const glm::vec2& coord);

//! Converts a raw pixel colour value to a floating point color, typically in range [0, 1]
glm::vec4 rawPixelToFloatColor(const void* rawData, Image::Format format);

//! Converts a floating point color, typically in range [0, 1], to a raw pixel colour value
void floatColorToRawPixel(const glm::vec4& color, void* rawData, Image::Format format);

unsigned int getPixelSizeBytes(Image::Format format);

} // namespace vis
} // namespace skybolt
