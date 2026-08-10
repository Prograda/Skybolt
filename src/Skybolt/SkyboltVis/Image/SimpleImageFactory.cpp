#include "SimpleImageFactory.h"
#include "ThirdParty/stb_image.h"
#include "ThirdParty/stb_image_write.h"
#include "ThirdParty/lodepng.h"

#include <algorithm>
#include <bit>
#include <cassert>
#include <cctype>
#include <cstdint>
#include <cstring>
#include <istream>
#include <iterator>
#include <map>
#include <memory>
#include <ostream>
#include <sstream>
#include <vector>


namespace skybolt::vis {

namespace {

size_t getBytesPerPixel(Image::Format format)
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
	assert(false);
	return 0;
}

std::string toLower(std::string value)
{
	std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
	return value;
}

Image::Format getFormatFromChannelCount(int channels)
{
	switch (channels)
	{
	case 1:
		return Image::Format::R8;
	case 3:
		return Image::Format::RGB8;
	case 4:
		return Image::Format::RGBA8;
	default:
		return Image::Format::Count;
	}
}

void stbWriteToOstream(void* context, void* data, int size)
{
	auto* stream = static_cast<std::ostream*>(context);
	stream->write(static_cast<const char*>(data), static_cast<std::streamsize>(size));
}

std::string toPngColorTypeString(LodePNGColorType colorType)
{
	switch (colorType)
	{
	case LCT_GREY:
		return "GREY";
	case LCT_RGB:
		return "RGB";
	case LCT_PALETTE:
		return "PALETTE";
	case LCT_GREY_ALPHA:
		return "GREY_ALPHA";
	case LCT_RGBA:
		return "RGBA";
	default:
		return "UNKNOWN";
	}
}

constexpr bool isNativeLittleEndian()
{
	return std::endian::native == std::endian::little;
}

void swapBytePairsInPlace(unsigned char* data, size_t byteCount)
{
	assert((byteCount % 2) == 0);
	for (size_t i = 0; i < byteCount; i += 2)
	{
		std::swap(data[i], data[i + 1]);
	}
}

void convertPng16BitSamplesToNativeInPlace(unsigned char* data, size_t byteCount)
{
	if constexpr (isNativeLittleEndian())
	{
		swapBytePairsInPlace(data, byteCount);
	}
}

std::vector<unsigned char> getPngCompatibleRawBytes(const Image& image)
{
	std::vector<unsigned char> bytes(image.getRawData(), image.getRawData() + image.getRawDataSizeBytes());
	if (image.getFormat() == Image::Format::R16 && isNativeLittleEndian())
	{
		swapBytePairsInPlace(bytes.data(), bytes.size());
	}
	return bytes;
}

Expected<ImagePtr> readPngImage(const std::vector<unsigned char>& encoded, const SimpleImageFactory& factory)
{
	lodepng::State state;
	unsigned width = 0;
	unsigned height = 0;
	unsigned error = lodepng_inspect(&width, &height, &state, encoded.data(), encoded.size());
	if (error != 0)
	{
		return UnexpectedMessage(lodepng_error_text(error));
	}

	Image::Format format = Image::Format::Count;
	const auto& color = state.info_png.color;
	if (color.colortype == LCT_GREY && color.bitdepth == 8)
	{
		format = Image::Format::R8;
	}
	else if (color.colortype == LCT_GREY && color.bitdepth == 16)
	{
		format = Image::Format::R16;
	}
	else if (color.colortype == LCT_RGB && color.bitdepth == 8)
	{
		format = Image::Format::RGB8;
	}
	else if (color.colortype == LCT_RGBA && color.bitdepth == 8)
	{
		format = Image::Format::RGBA8;
	}
	else
	{
		return UnexpectedMessage("Unsupported PNG format: color type:" + toPngColorTypeString(color.colortype) + ", bit depth: " + std::to_string(color.bitdepth));
	}

	state.info_raw = color;

	std::vector<unsigned char> decoded;
	error = lodepng::decode(decoded, width, height, state, encoded);
	if (error != 0)
	{
		return UnexpectedMessage(lodepng_error_text(error));
	}

	Expected<ImagePtr> image = factory.createImage(static_cast<int>(width), static_cast<int>(height), format, Image::ColorSpace::Srgb);
	if (!has_value(image))
	{
		return image;
	}

	const size_t expectedByteCount = static_cast<size_t>(width) * static_cast<size_t>(height) * getBytesPerPixel(format);
	if (decoded.size() != expectedByteCount)
	{
		return UnexpectedMessage("Decoded PNG byte count does not match expected image size");
	}

	if (format == Image::Format::R16)
	{
		convertPng16BitSamplesToNativeInPlace(decoded.data(), decoded.size());
	}

	std::memcpy((*value(image))->getRawData(), decoded.data(), decoded.size());
	return image;
}

Expected<std::monostate> writePngImage(const Image& image, std::ostream& s)
{
	lodepng::State state;
	state.encoder.auto_convert = 0; // Disable auto-convert to prevent lodepng from changing the color type or bit depth to something we don't support (e.g GREY_ALPHA 8-bit)
	switch (image.getFormat())
	{
	case Image::Format::R8:
		state.info_raw.colortype = LCT_GREY;
		state.info_raw.bitdepth = 8;
		state.info_png.color.colortype = LCT_GREY;
		state.info_png.color.bitdepth = 8;
		break;
	case Image::Format::R16:
		state.info_raw.colortype = LCT_GREY;
		state.info_raw.bitdepth = 16;
		state.info_png.color.colortype = LCT_GREY;
		state.info_png.color.bitdepth = 16;
		break;
	case Image::Format::RGB8:
		state.info_raw.colortype = LCT_RGB;
		state.info_raw.bitdepth = 8;
		state.info_png.color.colortype = LCT_RGB;
		state.info_png.color.bitdepth = 8;
		break;
	case Image::Format::RGBA8:
		state.info_raw.colortype = LCT_RGBA;
		state.info_raw.bitdepth = 8;
		state.info_png.color.colortype = LCT_RGBA;
		state.info_png.color.bitdepth = 8;
		break;
	default:
		return UnexpectedMessage("Failed to write PNG image because format is not supported: " + std::to_string((std::size_t)image.getFormat()));
	}

	const std::vector<unsigned char> rawBytes = getPngCompatibleRawBytes(image);
	std::vector<unsigned char> encoded;
	const unsigned error = lodepng::encode(
		encoded,
		rawBytes.data(),
		static_cast<unsigned>(image.getWidth()),
		static_cast<unsigned>(image.getHeight()),
		state);
	if (error != 0)
	{
		return UnexpectedMessage("Failed to encode PNG image");
	}

	s.write(reinterpret_cast<const char*>(encoded.data()), static_cast<std::streamsize>(encoded.size()));
	if (!s.good())
	{
		return UnexpectedMessage("Failed to write PNG image to stream");
	}
	return std::monostate{};
}

unsigned char toByte(float value)
{
	return static_cast<unsigned char>(std::round(glm::clamp(value, 0.0f, 1.0f) * 255.0f));
}

class SimpleImage final : public Image
{
public:
	SimpleImage(int width, int height, Format format, ColorSpace colorSpace) :
		mWidth(width),
		mHeight(height),
		mFormat(format),
		mColorSpace(colorSpace),
		mData(static_cast<size_t>(width) * static_cast<size_t>(height) * getBytesPerPixel(format), 0)
	{
	}

	int getWidth() const override { return mWidth; }
	int getHeight() const override { return mHeight; }

	const unsigned char* getRawData() const override { return mData.data(); }
	unsigned char* getRawData() override { return mData.data(); }

	glm::vec4 getColor(int x, int y) const override
	{
		const unsigned char* pixel = getPixelPtr(x, y);
		switch (mFormat)
		{
		case Format::R8:
			return glm::vec4(pixel[0] / 255.0f, 0.0f, 0.0f, 1.0f);
		case Format::R16:
		{
			uint16_t value;
			std::memcpy(&value, pixel, sizeof(value));
			return glm::vec4(value / 65535.0f, 0.0f, 0.0f, 1.0f);
		}
		case Format::R32F:
		{
			float value;
			std::memcpy(&value, pixel, sizeof(value));
			return glm::vec4(value, 0.0f, 0.0f, 1.0f);
		}
		case Format::RGB8:
			return glm::vec4(pixel[0] / 255.0f, pixel[1] / 255.0f, pixel[2] / 255.0f, 1.0f);
		case Format::RGBA8:
			return glm::vec4(pixel[0] / 255.0f, pixel[1] / 255.0f, pixel[2] / 255.0f, pixel[3] / 255.0f);
		case Format::Count:
			break;
		}
		assert(false);
		return glm::vec4(0.0f);
	}

	void setColor(int x, int y, const glm::vec4& color) override
	{
		unsigned char* pixel = getPixelPtr(x, y);
		switch (mFormat)
		{
		case Format::R8:
			pixel[0] = static_cast<unsigned char>(glm::clamp(color.r, 0.0f, 1.0f) * 255.0f);
			return;
		case Format::R16:
		{
			uint16_t value = static_cast<uint16_t>(glm::clamp(color.r, 0.0f, 1.0f) * 65535.0f);
			std::memcpy(pixel, &value, sizeof(value));
			return;
		}
		case Format::R32F:
		{
			float value = color.r;
			std::memcpy(pixel, &value, sizeof(value));
			return;
		}
		case Format::RGB8:
			pixel[0] = toByte(color.r);
			pixel[1] = toByte(color.g);
			pixel[2] = toByte(color.b);
			return;
		case Format::RGBA8:
			pixel[0] = toByte(color.r);
			pixel[1] = toByte(color.g);
			pixel[2] = toByte(color.b);
			pixel[3] = toByte(color.a);
			return;
		case Format::Count:
			break;
		}
		assert(false);
	}

	Format getFormat() const override { return mFormat; }

	ColorSpace getColorSpace() const override { return mColorSpace; }
	void setColorSpace(ColorSpace space) override { mColorSpace = space; }

	void addMetadata(const std::string& key, const ImageMetadataValue& value) override { mMetadata[key] = value; }
	void removeMetadata(const std::string& key) override { mMetadata.erase(key); }
	std::optional<ImageMetadataValue> getMetadata(const std::string& key) const override
	{
		auto it = mMetadata.find(key);
		if (it == mMetadata.end())
		{
			return std::nullopt;
		}
		return it->second;
	}

private:
	const unsigned char* getPixelPtr(int x, int y) const
	{
		assert(x >= 0 && x < mWidth);
		assert(y >= 0 && y < mHeight);
		return mData.data() + (static_cast<size_t>(y) * static_cast<size_t>(mWidth) + static_cast<size_t>(x)) * getBytesPerPixel(mFormat);
	}

	unsigned char* getPixelPtr(int x, int y)
	{
		assert(x >= 0 && x < mWidth);
		assert(y >= 0 && y < mHeight);
		return mData.data() + (static_cast<size_t>(y) * static_cast<size_t>(mWidth) + static_cast<size_t>(x)) * getBytesPerPixel(mFormat);
	}

	int mWidth;
	int mHeight;
	Format mFormat;
	ColorSpace mColorSpace;
	std::vector<unsigned char> mData;
	std::map<std::string, ImageMetadataValue> mMetadata;
};

} // namespace

Expected<ImagePtr> SimpleImageFactory::createImage(int width, int height, Image::Format format, Image::ColorSpace colorSpace) const
{
	if (width <= 0 || height <= 0)
	{
		return UnexpectedMessage("Invalid image dimensions: " + std::to_string(width) + "x" + std::to_string(height));
	}


	if (std::size_t(format) >= std::size_t(Image::Format::Count))
	{
		return UnexpectedMessage("Invalid image format: " + std::to_string((std::size_t)format));
	}

	return std::make_shared<SimpleImage>(width, height, format, colorSpace);
}

Expected<ImagePtr> SimpleImageFactory::readImage(std::istream& s, const std::string& formatExtension) const
{
	const std::string extension = toLower(formatExtension);

	const std::vector<unsigned char> encoded(
		(std::istreambuf_iterator<char>(s)),
		std::istreambuf_iterator<char>());

	if (encoded.empty())
	{
		return UnexpectedMessage("Image input stream is empty");
	}

	if (extension == ".png")
	{
		// Use lodepng instead as stb only handles 8-bit per channel PNGs
		return readPngImage(encoded, *this);
	}

	int width = 0;
	int height = 0;
	int channels = 0;
	std::shared_ptr<stbi_uc> decoded(
		stbi_load_from_memory(
			encoded.data(),
			static_cast<int>(encoded.size()),
			&width,
			&height,
			&channels,
			0),
		[](stbi_uc* data)
		{
			stbi_image_free(data);
		});

	if (!decoded)
	{
		return UnexpectedMessage(stbi_failure_reason());
	}

	const Image::Format format = getFormatFromChannelCount(channels);
	if (format == Image::Format::Count)
	{
		return UnexpectedMessage("Invalid image format: " + std::to_string((std::size_t)format));
	}

	Expected<ImagePtr> image = createImage(width, height, format, Image::ColorSpace::Srgb);
	if (!has_value(image))
	{
		return image;
	}

	const size_t byteCount = static_cast<size_t>(width) * static_cast<size_t>(height) * static_cast<size_t>(channels);
	std::memcpy((*value(image))->getRawData(), decoded.get(), byteCount);

	return image;
}

Expected<std::monostate> SimpleImageFactory::writeImage(const Image& image, std::ostream& s, const std::string& formatExtension) const
{
	const std::string extension = toLower(formatExtension);
	const int channels = Image::getChannelCount(image.getFormat());
	if (channels == 0)
	{
		return UnexpectedMessage("Failed to write image because it has zero color channels");
	}

	const int width = image.getWidth();
	const int height = image.getHeight();
	const unsigned char* data = image.getRawData();

	if (extension == ".png")
	{
		// Use lodepng instead as stb only handles 8-bit per channel PNGs
		return writePngImage(image, s);
	}

	if (extension == ".bmp")
	{
		if (!stbi_write_bmp_to_func(stbWriteToOstream, &s, width, height, channels, data) || !s.good())
		{
			return UnexpectedMessage("Failed to write BMP image to stream");
		}
		return std::monostate{};
	}

	if (extension == ".tga")
	{
		if (!stbi_write_tga_to_func(stbWriteToOstream, &s, width, height, channels, data) || !s.good())
		{
			return UnexpectedMessage("Failed to write TGA image to stream");
		}
		return std::monostate{};
	}

	if (extension == ".jpg" || extension == ".jpeg")
	{
		if (channels != 1 && channels != 3)
		{
			return UnexpectedMessage("Failed to write JPG image because it has " + std::to_string(channels) + " channels, but only 1 or 3 are supported");
		}

		if (stbi_write_jpg_to_func(stbWriteToOstream, &s, width, height, channels, data, 90) == 0)
		{
			return UnexpectedMessage("Failed to encode JPG image. Reason: " + std::string(stbi_failure_reason()));
		}

		if (!s.good())
		{
			return UnexpectedMessage("Failed to write JPG image to stream");
		}
		return std::monostate{};
	}

	return UnexpectedMessage("Failed to write unsupported image file format: " + extension);
}

} // namespace skybolt::vis
