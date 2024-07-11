/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "QtMenuUtil.h"

#include <QMenuBar>

QAction* findMenuWithText(const QMenuBar& bar, const QString& text)
{
	QAction* action = nullptr;
	for (QAction* action : bar.actions())
	{
		if (action->text() == text)
		{
			return action;
		}
	}
	return nullptr;
}

void insertMenuBefore(QMenuBar& bar, const QString& before, QMenu& menu)
{
	QAction* action = findMenuWithText(bar, before);
	action ? bar.insertMenu(action, &menu) : bar.addMenu(&menu);
}
