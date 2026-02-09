/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <QProxyStyle>

class DarkStyle : public QProxyStyle
{
 public:
	DarkStyle();
	explicit DarkStyle(QStyle* style);

	void polish(QPalette& palette) override;
	void polish(QApplication* app) override;
	QIcon standardIcon(QStyle::StandardPixmap standardIcon, const QStyleOption* option = nullptr, const QWidget* widget = nullptr) const override;
	QPixmap standardPixmap(StandardPixmap standardPixmap, const QStyleOption* option = nullptr, const QWidget* widget = nullptr) const override;
};
