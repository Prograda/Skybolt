#pragma once

class QAction;
class QMenu;
class QMenuBar;
class QString;

//! @returns nullptr if not found
QAction* findMenuWithText(const QMenuBar& bar, const QString& text);

void insertMenuBefore(QMenuBar& bar, const QString& before, QMenu& menu);
