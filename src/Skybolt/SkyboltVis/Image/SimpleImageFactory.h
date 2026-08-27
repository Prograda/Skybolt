#pragma once

#include "SkyboltVis/Image/ImageFactory.h"

namespace skybolt::vis {

class SimpleImageFactory : public ImageFactory
{
public:
	using ImageFactory::readImage;
	using ImageFactory::writeImage;

	~SimpleImageFactory() override = default;

	Expected<ImagePtr> createImage(int width, int height, Image::Format format, Image::ColorSpace colorSpace) const override;
	Expected<ImagePtr> readImage(std::istream& s, const std::string& formatExtension) const override;
	Expected<std::monostate> writeImage(const Image& image, std::ostream& s, const std::string& formatExtension) const override;
};

} // namespace skybolt::vis
