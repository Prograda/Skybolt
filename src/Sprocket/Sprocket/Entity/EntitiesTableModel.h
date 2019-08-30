/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <SkyboltSim/World.h>
#include <SkyboltEngine/SkyboltEngineFwd.h>
#include <QAbstractTableModel>

class EntitiesTableModel : public QAbstractTableModel, skybolt::sim::WorldListener
{
	Q_OBJECT
public:
	EntitiesTableModel(QObject *parent, skybolt::sim::World* world);
	~EntitiesTableModel();

	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	int columnCount(const QModelIndex &parent = QModelIndex()) const;
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
	QVariant headerData(int section, Qt::Orientation orientation, int role) const;

	skybolt::sim::Entity* getEntity(const QModelIndex &index) const;

private:
	void entityAdded(const skybolt::sim::EntityPtr& entity) override;
	void entityAboutToBeRemoved(const skybolt::sim::EntityPtr& entity) override;

private:
	skybolt::sim::World* mWorld;
	std::vector<skybolt::sim::Entity*> mEntities;
};
