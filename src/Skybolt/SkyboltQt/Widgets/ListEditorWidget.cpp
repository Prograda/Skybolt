#include "ListEditorWidget.h"
#include "Icon/SkyboltIcons.h"

#include <QBoxLayout>
#include <QDialogButtonBox>
#include <QToolBar>
#include <QToolButton>

ListEditorWidget::ListEditorWidget(QWidget* itemEditorWidget, QWidget* parent) :
	QWidget(parent)
{
	assert(itemEditorWidget);

	auto mainLayout = new QVBoxLayout(this);
	mainLayout->setContentsMargins(0, 0, 0, 0);

	// Create list controls
	{
		mListControlsWidget = new QToolBar(parent);
		mainLayout->addWidget(mListControlsWidget);

		mAddButton = new QToolButton(this);
		mAddButton->setIcon(getSkyboltIcon(SkyboltIcon::Add));
		mAddButton->setToolTip("Add");
		mListControlsWidget->addWidget(mAddButton);
		connect(mAddButton, &QAbstractButton::clicked, this, [this, mainLayout] {
			setAddItemModeEnabled(true);
			});

		mRemoveButton = new QToolButton(this);
		mRemoveButton->setIcon(getSkyboltIcon(SkyboltIcon::Remove));
		mRemoveButton->setToolTip("Remove");
		mRemoveButton->setEnabled(false);
		mListControlsWidget->addWidget(mRemoveButton);
		connect(mRemoveButton, &QAbstractButton::clicked, this, [this] {
			Q_EMIT itemRemoveRequested();
			});

		mMoveUpButton = new QToolButton(this);
		mMoveUpButton->setIcon(getSkyboltIcon(SkyboltIcon::MoveUp));
		mMoveUpButton->setToolTip("Move up");
		mMoveUpButton->setEnabled(false);
		mListControlsWidget->addWidget(mMoveUpButton);
		connect(mMoveUpButton, &QAbstractButton::clicked, this, [this] {
			Q_EMIT itemMoveUpRequested();
			});

		mMoveDownButton = new QToolButton(this);
		mMoveDownButton->setIcon(getSkyboltIcon(SkyboltIcon::MoveDown));
		mMoveDownButton->setToolTip("Move down");
		mMoveDownButton->setEnabled(false);
		mListControlsWidget->addWidget(mMoveDownButton);
		connect(mMoveDownButton, &QAbstractButton::clicked, this, [this] {
			Q_EMIT itemMoveDownRequested();
			});
	}

	// Create item editor
	{
		auto itemEditorLayout = new QVBoxLayout();
		mainLayout->addLayout(itemEditorLayout);

		itemEditorLayout->addWidget(itemEditorWidget);

		mCreateItemButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
		mCreateItemButtonBox->setVisible(false);
		itemEditorLayout->addWidget(mCreateItemButtonBox);

		connect(mCreateItemButtonBox, &QDialogButtonBox::accepted, this, [this] {
			setAddItemModeEnabled(false);
			Q_EMIT itemAddAccepted();
			});

		connect(mCreateItemButtonBox, &QDialogButtonBox::rejected, this, [this] {
			setAddItemModeEnabled(false);
			Q_EMIT itemAddCancelled();
			});
	}
}

void ListEditorWidget::setRemoveEnabled(bool enabled)
{
	mRemoveButton->setEnabled(enabled);
}

void ListEditorWidget::setMoveUpEnabled(bool enabled)
{
	mMoveUpButton->setEnabled(enabled);
}

void ListEditorWidget::setMoveDownEnabled(bool enabled)
{
	mMoveDownButton->setEnabled(enabled);
}

void ListEditorWidget::setAddItemModeEnabled(bool enabled)
{
	mListControlsWidget->setEnabled(!enabled);
	mCreateItemButtonBox->setVisible(enabled);

	if (enabled)
	{
		Q_EMIT itemAddInitiated();
	}
}
