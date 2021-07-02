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

IconFactory::IconFactory()
{
	registerIcon(Icon::Add, "Icons/google/add_circle.svg");
	registerIcon(Icon::Build, "Icons/google/build.svg");
	registerIcon(Icon::Code, "Icons/google/code.svg");
	registerIcon(Icon::FastForward, "Icons/google/round-fast_forward.svg");
	registerIcon(Icon::FastRewind, "Icons/google/round-fast_rewind.svg");
	registerIcon(Icon::Folder, "Icons/google/folder.svg");	
	registerIcon(Icon::Node, "Icons/google/brightness_1.svg");
	registerIcon(Icon::NodeGraph, "Icons/google/view_agenda.svg");
	registerIcon(Icon::Pause, "Icons/google/round-pause.svg");
	registerIcon(Icon::Play, "Icons/google/round-play_arrow.svg");
	registerIcon(Icon::Remove, "Icons/google/remove_circle.svg");
	registerIcon(Icon::Save, "Icons/google/save.svg");
	registerIcon(Icon::Sequence, "Icons/google/playlist_play.svg");
	registerIcon(Icon::Settings, "Icons/google/settings.svg");
}

QIcon IconFactory::createIcon(const Icon& icon) const
{
	auto it = m_icons.find(icon);
	if (it != m_icons.end())
	{
		return it->second;
	}
	return QIcon();
}

void colourizeImage(QImage& image, QColor color)
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

void IconFactory::registerIcon(const Icon& icon, const std::string& filename)
{
	file::Path locatedFile = locateFile(filename, file::FileLocatorMode::Required);
	if (locatedFile.empty())
	{
		return;
	}

	QFile file(QString::fromStdString(locatedFile.string()));
	if (!file.open(QIODevice::ReadOnly))
	{
		BOOST_LOG_TRIVIAL(error) << "Could not open file: " << locatedFile;
		return;
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

		m_icons[icon] = qicon;
	}
	file.close();
}

const IconFactory& getDefaultIconFactory()
{
	static IconFactory factory;
	return factory;
}
