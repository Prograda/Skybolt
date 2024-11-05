/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "StatusBar.h"
#include "ErrorLogModel.h"
#include "ErrorLogWidget.h"
#include "SkyboltQt/QtUtil/QtDialogUtil.h"

#include <QApplication>
#include <QBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QToolButton>
#include <QStatusBar>
#include <QStyle>

#include <assert.h>

void addErrorLogStatusBar(QStatusBar& bar, ErrorLogModel* model)
{
	assert(model);

	auto widget = new QWidget(&bar);
	auto layout = new QHBoxLayout(widget);
	layout->setMargin(0);
	
	auto label = new QLabel(&bar);
	label->setStyleSheet("QLabel { color : yellow; }");
	layout->addWidget(label);

	QFontMetrics fm(label->font());

	QStyle* style = QApplication::style();

	auto infoButton = new QToolButton(&bar);
	infoButton->setIcon(style->standardIcon(QStyle::SP_TitleBarNormalButton));
	infoButton->setToolTip("Expand");
	infoButton->setFixedHeight(fm.height());
	infoButton->setVisible(false);
	layout->addWidget(infoButton);

	QObject::connect(infoButton, &QToolButton::clicked, model, [parent = &bar, infoButton, model] {
		auto logWidget = new ErrorLogWidget(model, parent);
		QDialog* dialog = createDialogNonModal(logWidget, "Error Log");
		dialog->resize(800, 500);
		dialog->show();
	});

	bar.addPermanentWidget(widget);

    auto sink = [label, infoButton] (const ErrorLogModel::Item& item) {
		QString singleLineMessage = item.message;
		singleLineMessage = singleLineMessage.replace('\n', ' ');
		singleLineMessage.resize(std::min(singleLineMessage.size(), 100));
		singleLineMessage += "...";

		label->setText(singleLineMessage);
		infoButton->setVisible(!item.message.isEmpty());
	};

	QObject::connect(model, &ErrorLogModel::itemAppended, &bar, sink);
	if (!model->getItems().empty())
	{
		sink(model->getItems().back());
	}

	QObject::connect(model, &ErrorLogModel::cleared, &bar, [label, infoButton] {
		label->setText("");
		infoButton->setVisible(false);
	});
}
