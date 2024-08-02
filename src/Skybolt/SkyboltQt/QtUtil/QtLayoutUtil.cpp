/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "QtLayoutUtil.h"
#include <QGridLayout>
#include <QLabel>
#include <QWidget>

void clearLayout(QLayout& layout)
{
	while (QLayoutItem* item = layout.takeAt(0))
	{
		delete item->widget();
		if (item->layout())
		{
			clearLayout(*item->layout());
		}
		delete item;
	}
}

void addWidgetWithLabel(QGridLayout& layout, QWidget* widget, const QString& label)
{
	int row = layout.rowCount();
	layout.addWidget(new QLabel(label), row, 0);
	layout.addWidget(widget, row, 1);
}

QBoxLayout* createBoxLayoutWithWidgets(const std::vector<QWidget*>& widgets, QWidget* parent, QBoxLayout::Direction direction)
{
	auto layout = new QBoxLayout(direction, parent);
	for (QWidget* widget : widgets)
	{
		layout->addWidget(widget);
	}
	return layout;
}