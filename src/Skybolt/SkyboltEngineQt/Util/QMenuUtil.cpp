#include "QMenuUtil.h"

#include <QMenu>
#include <QAction>
#include <QList>
#include <algorithm>

void sortMenuRecursively(QMenu& menu)
{
	// Extract all current actions from the menu
	QList<QAction*> actions = menu.actions();

	// Safely remove actions from the menu without deleting them
	for (QAction* action : actions)
	{
		menu.removeAction(action);
	}

	//  Sort the actions alphabetically based on their text
	std::sort(actions.begin(), actions.end(), [](const QAction* a, const QAction* b) {
		// Compare text case-insensitively (removes '&' shortcuts for fair sorting if needed)
		QString textA = a->text().remove('&');
		QString textB = b->text().remove('&');
		return QString::compare(textA, textB, Qt::CaseInsensitive) < 0;
	});

	// Put the sorted actions back and recurse into submenus
	for (QAction* action : actions)
	{
		// Re-add the action to the cleared menu
		menu.addAction(action);

		// If this action opens a submenu, sort that submenu too
		if (QMenu* submenu = action->menu(); submenu)
		{
			sortMenuRecursively(*submenu);
		}
	}
}
