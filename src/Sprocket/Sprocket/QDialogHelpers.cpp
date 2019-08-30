/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "QDialogHelpers.h"
#include <QBoxLayout>
#include <QDialogButtonBox>
#include <memory>

std::shared_ptr<QDialog> createDialog(QWidget* content, const QString& title)
{
	auto dialog = std::shared_ptr<QDialog>(new QDialog(nullptr, Qt::WindowSystemMenuHint | Qt::WindowTitleHint), [](QObject *object) {
		object->deleteLater();
	});

	dialog->setWindowTitle(title);
	dialog->setLayout(new QVBoxLayout());
	dialog->layout()->addWidget(content);
	dialog->setModal(true);

	QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	dialog->layout()->addWidget(buttonBox);

	QObject::connect(buttonBox, &QDialogButtonBox::accepted, dialog.get(), &QDialog::accept);
	QObject::connect(buttonBox, &QDialogButtonBox::rejected, dialog.get(), &QDialog::reject);

	return dialog;
}
