/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "QtDialogUtil.h"
#include <QBoxLayout>
#include <QDialogButtonBox>
#include <QPushButton>
#include <memory>

std::shared_ptr<QDialog> createDialogModal(QWidget* content, const QString& title)
{
	auto dialog = std::shared_ptr<QDialog>(new QDialog(nullptr, Qt::WindowSystemMenuHint | Qt::WindowTitleHint), [](QObject *object) {
		object->deleteLater();
	});

	dialog->setWindowTitle(title);
	dialog->setLayout(new QVBoxLayout());
	dialog->layout()->addWidget(content);
	dialog->setModal(true);

	QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, dialog.get());
	dialog->layout()->addWidget(buttonBox);

	QObject::connect(buttonBox, &QDialogButtonBox::accepted, dialog.get(), &QDialog::accept);
	QObject::connect(buttonBox, &QDialogButtonBox::rejected, dialog.get(), &QDialog::reject);

	return dialog;
}

QDialog* createDialogNonModal(QWidget* content, const QString& title, QWidget* parent)
{
	auto dialog = new QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint);
	dialog->setWindowTitle(title);
	dialog->setLayout(new QVBoxLayout());
	dialog->layout()->addWidget(content);

	QPushButton* button = new QPushButton("Close");
	button->setAutoDefault(false);
	dialog->layout()->addWidget(button);
	QObject::connect(button, &QPushButton::pressed, dialog, &QDialog::accept);
	
	return dialog;
}
