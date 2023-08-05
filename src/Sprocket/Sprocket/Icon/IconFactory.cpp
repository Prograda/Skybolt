/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "IconFactory.h"
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

std::optional<QIcon> createIcon(const std::string& filename)
{
	file::Path locatedFile = locateFile(filename, file::FileLocatorMode::Required);
	if (locatedFile.empty())
	{
		BOOST_LOG_TRIVIAL(error) << "Could not find file: " << filename;
		return std::nullopt;
	}

	QFile file(QString::fromStdString(locatedFile.string()));
	if (!file.open(QIODevice::ReadOnly))
	{
		BOOST_LOG_TRIVIAL(error) << "Could not open file: " << locatedFile;
		return std::nullopt;
	}

	QColor color = Qt::lightGray;
	QColor disabledColor = Qt::darkGray;

	QPixmap pixmap;
	if (pixmap.loadFromData(file.readAll(), "SVG"))
	{
		QImage image = pixmap.toImage();
		colourizeImage(image, color);

		QIcon qicon = QPixmap::fromImage(image);

		colourizeImage(image, disabledColor);
		qicon.addPixmap(QPixmap::fromImage(image), QIcon::Mode::Disabled);

		return qicon;
	}
	else
	{
		BOOST_LOG_TRIVIAL(error) << "Could not read SVG file: " << locatedFile;
	}
	file.close();
	return std::nullopt;
}
