/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "QtScrollAreaUtil.h"
#include <QScrollArea>


QWidget* wrapWithVerticalScrollBar(QWidget* content, QWidget* parent)
{
	auto scrollArea = new QScrollArea(parent);
	scrollArea->setWidget(content);
	scrollArea->setWidgetResizable(true);
	scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	return scrollArea;
}