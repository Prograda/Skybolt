/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <QWidget>
#include <SkyboltEngine/SkyboltEngineFwd.h>

class QListWidget;

class EntityCreatorWidget : public QWidget
{
	Q_OBJECT
public:
	EntityCreatorWidget(QWidget* parent, skybolt::EntityFactory* factory);

private slots:
	void createEntity();

private:
	skybolt::EntityFactory* mFactory;
	QListWidget* mList;
};
