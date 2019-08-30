/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "SequenceListEditor.h"

#include <QPushButton>
#include <QBoxLayout>
#include <QInputDialog>
#include <QListWidget>
#include <QStandardItemModel>

#include <assert.h>

SequenceListEditor::SequenceListEditor(QStandardItemModel* model, QWidget* parent) :
	QWidget(parent)
{
	QListView* view = new QListView;
	view->setModel(model);

	QPushButton* createButton = new QPushButton("Create Sequence");
	connect(createButton, &QPushButton::pressed, [this, model]() {
		QString text = QInputDialog::getText(this, "Enter Name", "Enter the name of the new sequence");
		model->appendRow(new QStandardItem(text));
	});

	QPushButton* deleteButton = new QPushButton("Delete Sequence");
	deleteButton->setEnabled(false);
	connect(deleteButton, &QPushButton::pressed, [view, model]() {
		QModelIndex i = view->currentIndex();
		if (i.isValid())
		{
			model->removeRow(i.row());
		}
	});

	connect(view->selectionModel(), &QItemSelectionModel::selectionChanged, [deleteButton](const QItemSelection& newSelection, const QItemSelection& oldSelection) {
		deleteButton->setEnabled(!newSelection.isEmpty());
	});

	QHBoxLayout* buttonLayout = new QHBoxLayout;
	buttonLayout->addWidget(createButton);
	buttonLayout->addWidget(deleteButton);

	QVBoxLayout* mainLayout = new QVBoxLayout;
	mainLayout->addLayout(buttonLayout);
	mainLayout->addWidget(view);
	setLayout(mainLayout);
}