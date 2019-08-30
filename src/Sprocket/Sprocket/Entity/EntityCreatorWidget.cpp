/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "EntityCreatorWidget.h"

#include <SkyboltEngine/EntityFactory.h>
#include <QListWidget>
#include <QLayout>
#include <QPushButton>

using namespace skybolt;

EntityCreatorWidget::EntityCreatorWidget(QWidget* parent, EntityFactory* factory) :
	QWidget(parent),
	mFactory(factory)
{
	mList = new QListWidget(this);

	EntityFactory::Strings items = mFactory->getTemplateNames();
	for (const std::string& item : items)
	{
		mList->addItem(QString::fromStdString(item));
	}

	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->addWidget(mList);

	QPushButton* createButton = new QPushButton("Create", this);
	layout->addWidget(createButton);
	connect(createButton, SIGNAL(clicked()), this, SLOT(createEntity()));
}

void EntityCreatorWidget::createEntity()
{
	if (mList->currentItem())
	{
		std::string templateName = mList->currentItem()->text().toStdString();
		sim::Vector3 position(0, 0, 0);
		mFactory->createEntity(templateName, "", position);
	}
}
