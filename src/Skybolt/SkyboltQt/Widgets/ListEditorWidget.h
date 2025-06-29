#pragma once

#include <QWidget>
#include <variant>

class QDialogButtonBox;
class QToolBar;
class QToolButton;

class ListEditorWidget : public QWidget
{
	Q_OBJECT
public:
	ListEditorWidget(QWidget* itemEditorWidget, QWidget* parent = nullptr);

public Q_SLOTS:
	void setAddItemModeEnabled(bool enabled);
	void setRemoveEnabled(bool enabled);
	void setMoveUpEnabled(bool enabled);
	void setMoveDownEnabled(bool enabled);

Q_SIGNALS:
	void itemAddInitiated();
	void itemAddAccepted();
	void itemAddCancelled();

	void itemRemoveRequested();
	void itemMoveUpRequested();
	void itemMoveDownRequested();

private:
	QToolBar* mListControlsWidget;
	QToolButton* mAddButton;
	QToolButton* mRemoveButton;
	QToolButton* mMoveUpButton;
	QToolButton* mMoveDownButton;
	QDialogButtonBox* mCreateItemButtonBox;
};
