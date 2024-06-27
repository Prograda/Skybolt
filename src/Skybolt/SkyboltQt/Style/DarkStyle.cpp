/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "DarkStyle.h"

#include <QApplication>
#include <QFile>
#include <QFont>
#include <QStyleFactory>

static QStyle* getBaseStyle()
{
	static QStyle* base = QStyleFactory::create(QStringLiteral("Fusion"));
	return base;
}

DarkStyle::DarkStyle() :
	DarkStyle(getBaseStyle())
{
}

DarkStyle::DarkStyle(QStyle* style) :
	QProxyStyle(style)
{
}

void DarkStyle::polish(QPalette& palette)
{
	// Dark palette colors below are based on https://github.com/Jorgen-VikingGod/Qt-Frameless-Window-DarkStyle
	// Released under MIT License, Copyright (C) 2017 by Juergen Skrotzky (JorgenVikingGod@gmail.com)
	palette.setColor(QPalette::Window, QColor(53, 53, 53));
	palette.setColor(QPalette::WindowText, Qt::white);
	palette.setColor(QPalette::Disabled, QPalette::WindowText, QColor(127, 127, 127));
	palette.setColor(QPalette::Base, QColor(42, 42, 42));
	palette.setColor(QPalette::AlternateBase, QColor(66, 66, 66));
	palette.setColor(QPalette::ToolTipBase, Qt::white);
	palette.setColor(QPalette::ToolTipText, QColor(53, 53, 53));
	palette.setColor(QPalette::Text, Qt::white);
	palette.setColor(QPalette::Disabled, QPalette::Text, QColor(127, 127, 127));
	palette.setColor(QPalette::Dark, QColor(35, 35, 35));
	palette.setColor(QPalette::Shadow, QColor(20, 20, 20));
	palette.setColor(QPalette::Button, QColor(53, 53, 53));
	palette.setColor(QPalette::ButtonText, Qt::white);
	palette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(127, 127, 127));
	palette.setColor(QPalette::BrightText, Qt::red);
	palette.setColor(QPalette::Link, QColor(42, 130, 218));
	palette.setColor(QPalette::Highlight, QColor(42, 130, 218));
	palette.setColor(QPalette::Disabled, QPalette::Highlight, QColor(80, 80, 80));
	palette.setColor(QPalette::HighlightedText, Qt::white);
	palette.setColor(QPalette::Disabled, QPalette::HighlightedText, QColor(127, 127, 127));
}

void DarkStyle::polish(QApplication* app)
{
	if (!app)
	{
		return;
	}

	// Increase font size
	QFont defaultFont = QApplication::font();
	defaultFont.setPointSize(defaultFont.pointSize() + 1);
	app->setFont(defaultFont);

	// Load stylesheet
	QFile stylesheetFile(QStringLiteral(":/darkstyle/darkstyle.qss"));
	if (stylesheetFile.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		QString qsStylesheet = QString::fromLatin1(stylesheetFile.readAll());
		app->setStyleSheet(qsStylesheet);
		stylesheetFile.close();
	}
}

QIcon DarkStyle::standardIcon(QStyle::StandardPixmap standardIcon, const QStyleOption* option, const QWidget* widget) const
{
	switch (standardIcon)
	{
		case SP_TitleBarNormalButton:
		{
			static QIcon icon(":/darkstyle/icon_restore.png");
			return icon;
		}
		case SP_TitleBarCloseButton:
		case SP_DockWidgetCloseButton:
		case SP_DialogCloseButton:
		{
			static QIcon icon(":/darkstyle/icon_close.png");
			return icon;
		}
	}

	return QProxyStyle::standardIcon(standardIcon, option, widget);
}

QPixmap DarkStyle::standardPixmap(QStyle::StandardPixmap standardPixmap, const QStyleOption* option, const QWidget* widget) const
{
	switch (standardPixmap)
	{
		case SP_TitleBarNormalButton:
		{
			static QPixmap pixmap(":/darkstyle/icon_restore.png");
			return pixmap;
		}
		case SP_TitleBarCloseButton:
		case SP_DockWidgetCloseButton:
		case SP_DialogCloseButton:
		{
			static QPixmap pixmap(":/darkstyle/icon_close.png");
			return pixmap;
		}
	}

	return QProxyStyle::standardPixmap(standardPixmap, option, widget);
}