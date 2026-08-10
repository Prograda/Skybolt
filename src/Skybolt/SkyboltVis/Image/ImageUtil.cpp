/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "ImageUtil.h"
#include "Image.h"
#include <SkyboltCommon/Math/MathUtility.h>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string.hpp>
#include <assert.h>
#include <fstream>

namespace skybolt {
namespace vis {

glm::vec4 applyGamma(const glm::vec4& c, float gamma)
{
	glm::vec4 r = c;
	r.r = std::pow(c.r, gamma);
	r.g = std::pow(c.g, gamma);
	r.b = std::pow(c.b, gamma);
	return r;
}

glm::vec4 averageSrgbColor(const Image& image, float alphaRejectionThreshold)
{
	static const float gamma = srgbGamma();
	glm::vec4 color;
	size_t pixels = 0;
	for (int t = 0; t < image.getHeight(); ++t)
	{
		for (int s = 0; s < image.getWidth(); ++s)
		{
			glm::vec4 c = image.getColor(s, t);
			if (c.a >= alphaRejectionThreshold)
			{
				color += srgbToLinear(c);
				++pixels;
			}
		}
	}

	color /= pixels;

	return linearToSrgb(color);
}

glm::vec4 getColorBilinear(const Image& image, const glm::vec2& coord)
{
	// Calculate coordinates
	int sMax = image.getWidth() - 1;
	int tMax = image.getHeight() - 1;

	glm::vec2 clampedCoord;
	clampedCoord.x = math::clamp(coord.x, 0.0f, float(sMax));
	clampedCoord.y = math::clamp(coord.y, 0.0f, float(tMax));

	int u0 = (int)clampedCoord.x;
	int u1 = std::min(u0 + 1, sMax);
	int v0 = (int)clampedCoord.y;
	int v1 = std::min(v0 + 1, tMax);

	// Calculate weights
	float fracU = clampedCoord.x - u0;
	float fracV = clampedCoord.y - v0;

	// Interpolate
	glm::vec4 d00 = image.getColor(u0, v0);
	glm::vec4 d10 = image.getColor(u1, v0);
	glm::vec4 d01 = image.getColor(u0, v1);
	glm::vec4 d11 = image.getColor(u1, v1);

	glm::vec4 fracUVec(fracU, fracU, fracU, fracU);
	glm::vec4 d0 = math::componentWiseLerp(d00, d10, fracUVec);
	glm::vec4 d1 = math::componentWiseLerp(d01, d11, fracUVec);

	return math::componentWiseLerp(d0, d1, glm::vec4(fracV, fracV, fracV, fracV));
}

static glm::vec4 componentWiseMin(const glm::vec4& a, const glm::vec4& b)
{
	glm::vec4 r;
	for (int i = 0; i < 4; ++i)
	{
		r[i] = std::min(a[i], b[i]);
	}
	return r;
}

static glm::vec4 componentWiseMax(const glm::vec4& a, const glm::vec4& b)
{
	glm::vec4 r;
	for (int i = 0; i < 4; ++i)
	{
		r[i] = std::max(a[i], b[i]);
	}
	return r;
}

void normalize(Image& image)
{
	constexpr float inf = std::numeric_limits<float>::infinity();
	glm::vec4 cMin(inf, inf, inf, inf);
	glm::vec4 cMax(0, 0, 0, 0);

	for (int y = 0; y < image.getHeight(); ++y)
	{
		for (int x = 0; x < image.getWidth(); ++x)
		{
			glm::vec4 c = image.getColor(x, y);
			cMin = componentWiseMin(c, cMin);
			cMax = componentWiseMax(c, cMax);
		}
	}

	for (int y = 0; y < image.getHeight(); ++y)
	{
		for (int x = 0; x < image.getWidth(); ++x)
		{
			glm::vec4 c = image.getColor(x, y);
			c = ((c - cMin) / (cMax - cMin));
			image.setColor(x, y, c);
		}
	}
}

void flipVertical(Image& image)
{
	const int width = image.getWidth();
	const int height = image.getHeight();
	const std::size_t rowSizeBytes = static_cast<std::size_t>(width) * Image::getBytesPerPixel(image.getFormat());
	unsigned char* rawData = image.getRawData();
	std::vector<unsigned char> tempRow(rowSizeBytes);

	for (int y = 0; y < height / 2; ++y)
	{
		unsigned char* topRow = rawData + static_cast<std::size_t>(y) * rowSizeBytes;
		unsigned char* bottomRow = rawData + static_cast<std::size_t>(height - 1 - y) * rowSizeBytes;

		std::memcpy(tempRow.data(), topRow, rowSizeBytes);
		std::memcpy(topRow, bottomRow, rowSizeBytes);
		std::memcpy(bottomRow, tempRow.data(), rowSizeBytes);
	}
}

glm::vec4 getColorAtNormCoord(const Image& image, const glm::vec2& coord)
{
	return image.getColor(
		int(coord.x * float(image.getWidth())),
		int(coord.y * float(image.getHeight())));
}

glm::vec4 rawPixelToFloatColor(const void* rawData, Image::Format format)
{
	assert(rawData);

	switch (format)
	{
	case Image::Format::R16:
		return glm::vec4(float(*static_cast<const std::uint16_t*>(rawData)) / 65535.f, 0, 0, 1.f);
	case Image::Format::R32F:
		return glm::vec4(*static_cast<const float*>(rawData), 0, 0, 1.f);
	case Image::Format::R8:
		return glm::vec4(float(*static_cast<const std::uint8_t*>(rawData)) / 255.f, 0, 0, 1.f);
	case Image::Format::RGB8:
	{
		auto p = static_cast<const std::uint8_t*>(rawData);
		return glm::vec4(float(p[0]) / 255.f, float(p[1]) / 255.f, float(p[2]) / 255.f, 1.f);
	}
	case Image::Format::RGBA8:
	{
		auto p = static_cast<const std::uint8_t*>(rawData);
		return glm::vec4(float(p[0]) / 255.f, float(p[1]) / 255.f, float(p[2]) / 255.f, float(p[3]) / 255.f);
	}
	}
	assert(!"Not implemented");
	return glm::vec4{};
}

//! Converts a floating point color, typically in range [0, 1], to a raw pixel colour value
void floatColorToRawPixel(const glm::vec4& color, void* rawData, Image::Format format)
{
	assert(rawData);

	// We use std::round() when converting from integer color to float because it is the most accurate method.
	// See: https://stackoverflow.com/questions/1914115/converting-color-value-from-float-0-1-to-byte-0-255

	switch (format)
	{
	case Image::Format::R16:
		*static_cast<std::uint16_t*>(rawData) = std::uint16_t(std::round(color.r * 65535.f));
		return;
	case Image::Format::R32F:
		*static_cast<float*>(rawData) = color.r;
		return;
	case Image::Format::R8:
		*static_cast<std::uint8_t*>(rawData) = std::uint8_t(std::round(color.r * 255.f));
		return;
	case Image::Format::RGB8:
	{
		auto p = static_cast<std::uint8_t*>(rawData);
		p[0] = std::uint8_t(std::round(color.r * 255.f));
		p[1] = std::uint8_t(std::round(color.g * 255.f));
		p[2] = std::uint8_t(std::round(color.b * 255.f));
		return;
	}
	case Image::Format::RGBA8:
	{
		auto p = static_cast<std::uint8_t*>(rawData);
		p[0] = std::uint8_t(std::round(color.r * 255.f));
		p[1] = std::uint8_t(std::round(color.g * 255.f));
		p[2] = std::uint8_t(std::round(color.b * 255.f));
		p[3] = std::uint8_t(std::round(color.a * 255.f));
		return;
	}
	}
	assert(!"Not implemented");
}

unsigned int getPixelSizeBytes(Image::Format format)
{
	switch (format)
	{
	case Image::Format::R16: return 2;
	case Image::Format::R32F: return 4;
	case Image::Format::R8: return 1;
	case Image::Format::RGB8: return 3;
	case Image::Format::RGBA8: return 4;
	}
	assert(!"Not implemented");
	return 0;
}

} // namespace vis
} // namespace skybolt
