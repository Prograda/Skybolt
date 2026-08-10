#pragma once

#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <glm/glm.hpp>

namespace skybolt::vis {

using ImageMetadataValue = std::variant<int, float, double, glm::vec4>;

class Image
{
public:
	virtual ~Image() = default;

	enum class Format
	{
		R8,
		R16,
		R32F,
		RGB8,
		RGBA8,
		Count,
	};

	enum class ColorSpace
	{
		Linear,
		Srgb
	};
	
	static int getChannelCount(Image::Format format)
	{
		switch (format)
		{
		case Image::Format::R8:
		case Image::Format::R16:
		case Image::Format::R32F:
			return 1;
		case Image::Format::RGB8:
			return 3;
		case Image::Format::RGBA8:
			return 4;
		case Image::Format::Count:
			break;
		}
		return 0;
	}

	static int getBytesPerPixel(Image::Format format)
	{
		switch (format)
		{
		case Image::Format::R8:
			return 1;
		case Image::Format::R16:
			return 2;
		case Image::Format::R32F:
			return 4;
		case Image::Format::RGB8:
			return 3;
		case Image::Format::RGBA8:
			return 4;
		case Image::Format::Count:
			break;
		}
		return 0;
	}

	virtual int getWidth() const = 0;
	virtual int getHeight() const = 0;

	//! Raw data is encoded in row-major order, with the first pixel being the top-left pixel of the image.
	virtual const unsigned char* getRawData() const = 0;
	virtual unsigned char* getRawData() = 0;

	std::size_t getRawDataSizeBytes() const { return static_cast<std::size_t>(getWidth()) * static_cast<std::size_t>(getHeight()) * getBytesPerPixel(getFormat()); }

	//! @returns floating point color (typically in range [0, 1]) at the given coordinates
	virtual glm::vec4 getColor(int x, int y) const = 0;

	//! Sets floating point color (typically in range [0, 1]) at the given coordinates
	virtual void setColor(int x, int y, const glm::vec4& color) = 0;

	virtual Format getFormat() const = 0;

	virtual ColorSpace getColorSpace() const = 0;
	virtual void setColorSpace(ColorSpace space) = 0;

	virtual void addMetadata(const std::string& key, const ImageMetadataValue& value) = 0;
	virtual void removeMetadata(const std::string& key) = 0;
	virtual std::optional<ImageMetadataValue> getMetadata(const std::string& key) const = 0;
};

} // namespace skybolt::vis