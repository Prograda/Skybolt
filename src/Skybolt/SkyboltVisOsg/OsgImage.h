/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <SkyboltVis/Image/ImageFactory.h>
#include <osg/Image>

#include <map>

namespace skybolt {
namespace vis {

class OsgImage : public Image
{
public:
	explicit OsgImage(osg::ref_ptr<osg::Image> image, ColorSpace colorSpace = ColorSpace::Linear);

	const osg::ref_ptr<osg::Image>& getOsgImage() const { return mImage; }
	osg::ref_ptr<osg::Image>& getOsgImage() { return mImage; }

	int getWidth() const override;
	int getHeight() const override;

	const unsigned char* getRawData() const override;
	unsigned char* getRawData() override;

	glm::vec4 getColor(int x, int y) const override;
	void setColor(int x, int y, const glm::vec4& color) override;

	Format getFormat() const override;

	ColorSpace getColorSpace() const override;
	void setColorSpace(ColorSpace space) override;

	void addMetadata(const std::string& key, const ImageMetadataValue& value) override;
	void removeMetadata(const std::string& key) override;
	std::optional<ImageMetadataValue> getMetadata(const std::string& key) const override;

private:
	osg::ref_ptr<osg::Image> mImage;
	ColorSpace mColorSpace;
	std::map<std::string, ImageMetadataValue> mMetadata;
};

class OsgImageFactory final : public ImageFactory
{
public:
	using ImageFactory::readImage;
	using ImageFactory::writeImage;

	~OsgImageFactory() override = default;

	Expected<ImagePtr> createImage(int width, int height, Image::Format format, Image::ColorSpace colorSpace) const override;
	Expected<ImagePtr> readImage(std::istream& s, const std::string& formatExtension) const override;
	Expected<std::monostate> writeImage(const Image& image, std::ostream& s, const std::string& formatExtension) const override;
};

} // namespace vis
} // namespace skybolt
