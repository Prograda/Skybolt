#include <catch2/catch.hpp>

#include "SkyboltVis/Image/SimpleImageFactory.h"

#include <cstring>
#include <filesystem>
#include <sstream>

namespace skybolt::vis {

TEST_CASE("SimpleImageFactory creates basic in-memory images")
{
	SimpleImageFactory factory;

	ImagePtr image = valueOrThrowException(factory.createImage(2, 3, Image::Format::RGBA8, Image::ColorSpace::Srgb));
	REQUIRE(image);
	CHECK(image->getWidth() == 2);
	CHECK(image->getHeight() == 3);
	CHECK(image->getFormat() == Image::Format::RGBA8);
	CHECK(image->getColorSpace() == Image::ColorSpace::Srgb);

	image->setColor(1, 2, glm::vec4(0.25f, 0.5f, 0.75f, 1.0f));
	glm::vec4 color = image->getColor(1, 2);
	CHECK(color.r == Approx(0.25f).margin(0.01f));
	CHECK(color.g == Approx(0.5f).margin(0.01f));
	CHECK(color.b == Approx(0.75f).margin(0.01f));
	CHECK(color.a == Approx(1.0f).margin(0.01f));

	image->addMetadata("answer", 42);
	REQUIRE(image->getMetadata("answer").has_value());
	CHECK(std::get<int>(*image->getMetadata("answer")) == 42);
}

TEST_CASE("SimpleImageFactory rejects invalid image dimensions")
{
	SimpleImageFactory factory;
	CHECK(!has_value(factory.createImage(0, 1, Image::Format::R8, Image::ColorSpace::Linear)));
	CHECK(!has_value(factory.createImage(1, 0, Image::Format::R8, Image::ColorSpace::Linear)));
	CHECK(!has_value(factory.createImage(1, 1, Image::Format::Count, Image::ColorSpace::Linear)));
}

TEST_CASE("SimpleImageFactory round-trips PNG images through streams")
{
	SimpleImageFactory factory;
	ImagePtr image = valueOrThrowException(factory.createImage(2, 2, Image::Format::RGBA8, Image::ColorSpace::Srgb));

	image->setColor(0, 0, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
	image->setColor(1, 0, glm::vec4(0.0f, 1.0f, 0.0f, 0.5f));
	image->setColor(0, 1, glm::vec4(0.0f, 0.0f, 1.0f, 0.25f));
	image->setColor(1, 1, glm::vec4(1.0f, 1.0f, 0.0f, 0.75f));

	std::stringstream stream(std::ios::in | std::ios::out | std::ios::binary);
	valueOrThrowException(factory.writeImage(*image, stream, ".png"));

	stream.seekg(0);
	ImagePtr decoded = valueOrThrowException(factory.readImage(stream, ".png"));

	CHECK(decoded->getWidth() == 2);
	CHECK(decoded->getHeight() == 2);
	CHECK(decoded->getFormat() == Image::Format::RGBA8);
	CHECK(decoded->getColorSpace() == Image::ColorSpace::Srgb);

	CHECK(decoded->getColor(0, 0).r == Approx(1.0f).margin(0.01f));
	CHECK(decoded->getColor(0, 0).g == Approx(0.0f).margin(0.01f));
	CHECK(decoded->getColor(0, 0).b == Approx(0.0f).margin(0.01f));
	CHECK(decoded->getColor(0, 0).a == Approx(1.0f).margin(0.01f));

	CHECK(decoded->getColor(1, 0).r == Approx(0.0f).margin(0.01f));
	CHECK(decoded->getColor(1, 0).g == Approx(1.0f).margin(0.01f));
	CHECK(decoded->getColor(1, 0).b == Approx(0.0f).margin(0.01f));
	CHECK(decoded->getColor(1, 0).a == Approx(0.5f).margin(0.01f));

	CHECK(decoded->getColor(0, 1).r == Approx(0.0f).margin(0.01f));
	CHECK(decoded->getColor(0, 1).g == Approx(0.0f).margin(0.01f));
	CHECK(decoded->getColor(0, 1).b == Approx(1.0f).margin(0.01f));
	CHECK(decoded->getColor(0, 1).a == Approx(0.25f).margin(0.01f));

	CHECK(decoded->getColor(1, 1).r == Approx(1.0f).margin(0.01f));
	CHECK(decoded->getColor(1, 1).g == Approx(1.0f).margin(0.01f));
	CHECK(decoded->getColor(1, 1).b == Approx(0.0f).margin(0.01f));
	CHECK(decoded->getColor(1, 1).a == Approx(0.75f).margin(0.01f));
}

TEST_CASE("SimpleImageFactory round-trips images through files")
{
	SimpleImageFactory factory;
	ImagePtr image = valueOrThrowException(factory.createImage(2, 2, Image::Format::RGB8, Image::ColorSpace::Srgb));

	image->setColor(0, 0, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
	image->setColor(1, 0, glm::vec4(0.0f, 1.0f, 0.0f, 0.5f));
	image->setColor(0, 1, glm::vec4(0.0f, 0.0f, 1.0f, 0.25f));
	image->setColor(1, 1, glm::vec4(1.0f, 1.0f, 0.0f, 0.75f));

	const auto tempDirectory = std::filesystem::temp_directory_path() / "SkyboltTests";
	std::filesystem::create_directories(tempDirectory);

	const std::string formats[] = { ".png", ".bmp" };

	for (const auto& extension : formats)
	{
		const auto filename = tempDirectory / (std::string("SimpleImageFactory_roundtrip") + extension);

		CAPTURE(extension);
		valueOrThrowException(factory.writeImage(*image, filename.string()));
		REQUIRE(std::filesystem::exists(filename));

		ImagePtr decoded = valueOrThrowException(factory.readImage(filename.string()));
		CHECK(decoded->getWidth() == image->getWidth());
		CHECK(decoded->getHeight() == image->getHeight());
		CHECK(decoded->getFormat() == image->getFormat());
		CHECK(decoded->getColorSpace() == image->getColorSpace());

		CHECK(decoded->getColor(0, 0).r == Approx(1.0f).margin(0.05f));
		CHECK(decoded->getColor(0, 0).g == Approx(0.0f).margin(0.05f));
		CHECK(decoded->getColor(0, 0).b == Approx(0.0f).margin(0.05f));

		CHECK(decoded->getColor(1, 0).r == Approx(0.0f).margin(0.05f));
		CHECK(decoded->getColor(1, 0).g == Approx(1.0f).margin(0.05f));
		CHECK(decoded->getColor(1, 0).b == Approx(0.0f).margin(0.05f));

		CHECK(decoded->getColor(0, 1).r == Approx(0.0f).margin(0.05f));
		CHECK(decoded->getColor(0, 1).g == Approx(0.0f).margin(0.05f));
		CHECK(decoded->getColor(0, 1).b == Approx(1.0f).margin(0.05f));

		CHECK(decoded->getColor(1, 1).r == Approx(1.0f).margin(0.05f));
		CHECK(decoded->getColor(1, 1).g == Approx(1.0f).margin(0.05f));
		CHECK(decoded->getColor(1, 1).b == Approx(0.0f).margin(0.05f));

		std::filesystem::remove(filename);
	}
}

TEST_CASE("SimpleImageFactory round-trips 16-bit PNG image")
{
	SimpleImageFactory factory;
	ImagePtr image = valueOrThrowException(factory.createImage(2, 2, Image::Format::R16, Image::ColorSpace::Srgb));

	const uint16_t expectedSamples[] = {
		0u,
		1u,
		32768u,
		65535u
	};
	std::memcpy(image->getRawData(), expectedSamples, sizeof(expectedSamples));

	std::stringstream stream(std::ios::in | std::ios::out | std::ios::binary);
	valueOrThrowException(factory.writeImage(*image, stream, ".png"));

	stream.seekg(0);
	ImagePtr decoded = valueOrThrowException(factory.readImage(stream, ".png"));

	REQUIRE(decoded);
	CHECK(decoded->getWidth() == 2);
	CHECK(decoded->getHeight() == 2);
	CHECK(decoded->getFormat() == Image::Format::R16);
	CHECK(decoded->getColorSpace() == Image::ColorSpace::Srgb);
	CHECK(decoded->getRawDataSizeBytes() == sizeof(expectedSamples));

	const auto* decodedSamples = reinterpret_cast<const uint16_t*>(decoded->getRawData());
	for (size_t i = 0; i < std::size(expectedSamples); ++i)
	{
		CHECK(decodedSamples[i] == expectedSamples[i]);
	}
}

} // namespace skybolt::vis
