/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "IconFactory.h"

#define NANOSVG_IMPLEMENTATION
#define NANOSVGRAST_IMPLEMENTATION
#include "ThirdParty/nanosvg/nanosvg.h"
#include "ThirdParty/nanosvg/nanosvgrast.h"

#include <SkyboltEngine/EngineRoot.h>

#include <boost/log/trivial.hpp>
#include <QBitmap>
#include <QFile>
#include <QApplication>
#include <QStyle>
#include <QStyleOption>

using namespace skybolt;

static void colourizeImage(QImage& image, QColor color)
{
	for (int y = 0; y < image.height(); y++)
	{
		for (int x = 0; x < image.width(); x++)
		{
			color.setAlpha(image.pixelColor(x, y).alpha());
			image.setPixelColor(x, y, color);
		}
	}
}

//! Load SVG using Qt. Requres the Qt SVG plugin.
//! Use loadSvgWithNanoSvg instead if you don't want to use the Qt SVG plugin.
static std::optional<QPixmap> loadSvgWithQt(const file::Path& filename)
{
	QFile file(QString::fromStdString(filename.string()));
	if (!file.open(QIODevice::ReadOnly))
	{
		BOOST_LOG_TRIVIAL(error) << "Could not open file: " << filename;
		return std::nullopt;
	}

	QPixmap pixmap;
	if (pixmap.loadFromData(file.readAll(), "SVG"))
	{
		file.close();
		return pixmap;
	}
	else
	{
		BOOST_LOG_TRIVIAL(error) << "Could not read SVG file: " << filename;
	}
	file.close();
	return std::nullopt;
}

//! Loads SVG using Nano SVG. This is useful to avoid needing to compile the Qt SVG plugin,
//! which is not included in the pre-built Qt conan packages.
static std::optional<QPixmap> loadSvgWithNanoSvg(const file::Path& filename)
{
	int iconSize = 24;

	if (NSVGimage* image = nsvgParseFromFile(filename.string().c_str(), "px", 96); image)
	{
		static auto rast = std::shared_ptr<NSVGrasterizer>(nsvgCreateRasterizer(), [] (NSVGrasterizer* r) {
			nsvgDeleteRasterizer(r);
			});

		float scale = float(iconSize) / float(image->width);
		std::vector<std::uint8_t> data(iconSize * iconSize * 4);
		nsvgRasterize(rast.get(), image, 0,0,scale, data.data(), iconSize, iconSize, iconSize*4);
		nsvgDelete(image);

		return QPixmap::fromImage(QImage(data.data(), iconSize, iconSize, QImage::Format_RGBA8888));
	}
	else
	{
		BOOST_LOG_TRIVIAL(error) << "Could not read SVG file: " << filename;
	}
	return std::nullopt;
}

std::optional<QIcon> createIcon(const std::string& filename)
{
	file::Path locatedFile = locateFile(filename, file::FileLocatorMode::Required);
	if (locatedFile.empty())
	{
		BOOST_LOG_TRIVIAL(error) << "Could not find file: " << filename;
		return std::nullopt;
	}

	QColor color = Qt::lightGray;
	QColor disabledColor = Qt::darkGray;

	if (std::optional<QPixmap> pixmap = loadSvgWithNanoSvg(locatedFile); pixmap)
	{
		QImage image = pixmap->toImage();
		colourizeImage(image, color);

		QIcon qicon = QPixmap::fromImage(image);

		colourizeImage(image, disabledColor);
		qicon.addPixmap(QPixmap::fromImage(image), QIcon::Mode::Disabled);

		return qicon;
	}
	return std::nullopt;
}
