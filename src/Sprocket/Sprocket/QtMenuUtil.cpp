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
