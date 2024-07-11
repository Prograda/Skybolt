/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "QtLayoutUtil.h"
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