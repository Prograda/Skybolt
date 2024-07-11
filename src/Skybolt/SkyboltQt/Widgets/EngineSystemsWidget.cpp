/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "EngineSystemsWidget.h"

#include <SkyboltSim/System/System.h>

#include <QTextEdit>

QWidget* createEngineSystemsWidget(const skybolt::sim::SystemRegistry& registry, QWidget* parent)
{
	QString str;
	for (const auto& system : registry)
	{
		str += QString(typeid(*system).name()) + "\n";
	}

	auto textEdit = new QTextEdit(parent);
	textEdit->setPlainText(str);
	textEdit->setReadOnly(true);
	return textEdit;
}