/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "IconFactory.h"
#include <QBitmap>
#include <QFile>
#include <QApplication>
#include <QStyle>
#include <QStyleOption>

IconFactory::IconFactory()
{
	registerIcon(Icon::Add, "Assets/Core/Icons/google/add_circle.svg");
	registerIcon(Icon::Build, "Assets/Core/Icons/google/build.svg");
	registerIcon(Icon::Code, "Assets/Core/Icons/google/code.svg");
	registerIcon(Icon::FastForward, "Assets/Core/Icons/google/round-fast_forward.svg");
	registerIcon(Icon::FastRewind, "Assets/Core/Icons/google/round-fast_rewind.svg");
	registerIcon(Icon::Folder, "Assets/Core/Icons/google/folder.svg");	
	registerIcon(Icon::Node, "Assets/Core/Icons/google/brightness_1.svg");
	registerIcon(Icon::NodeGraph, "Assets/Core/Icons/google/view_agenda.svg");
	registerIcon(Icon::Pause, "Assets/Core/Icons/google/round-pause.svg");
	registerIcon(Icon::Play, "Assets/Core/Icons/google/round-play_arrow.svg");
	registerIcon(Icon::Remove, "Assets/Core/Icons/google/remove_circle.svg");
	registerIcon(Icon::Save, "Assets/Core/Icons/google/save.svg");
	registerIcon(Icon::Sequence, "Assets/Core/Icons/google/playlist_play.svg");
	registerIcon(Icon::Settings, "Assets/Core/Icons/google/settings.svg");
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

void IconFactory::registerIcon(const Icon& icon, const QString& filename)
{
	QFile file(filename);
	file.open(QIODevice::ReadOnly);

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
}

const IconFactory& getDefaultIconFactory()
{
	static IconFactory factory;
	return factory;
}
