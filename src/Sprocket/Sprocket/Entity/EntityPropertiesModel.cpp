/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "EntityPropertiesModel.h"
#include "Sprocket/Property/QtPropertyReflection.h"
#include <SkyboltSim/Components/NameComponent.h>
#include <SkyboltSim/Entity.h>

#include <boost/scope_exit.hpp>

using namespace skybolt;

EntityPropertiesModel::EntityPropertiesModel(sim::Entity* entity) :
	mEntity(nullptr)
{
	setEntity(entity);
}

EntityPropertiesModel::~EntityPropertiesModel()
{
	if (mEntity)
		mEntity->removeListener(this);
}

void EntityPropertiesModel::setEntity(sim::Entity* entity)
{
	mProperties.clear();

	if (mEntity)
		mEntity->removeListener(this);

	mEntity = entity;

	if (mEntity)
	{
		mEntity->addListener(this);

		addProperty(createVariantProperty(QLatin1String("name"), QString()), [this](QtProperty& property) {
			static_cast<VariantProperty&>(property).setValue(QString::fromStdString(getName(*mEntity)));
		});

		for (const sim::ComponentPtr& component : mEntity->getComponents())
		{
			RttrInstanceGetter getter = [component] { return component; };
			addRttrPropertiesToModel(*this, sim::getProperties(*component), getter);
		}

		addProperty(createVariantProperty("dynamicsEnabled", false),
			// Updater
			[this](QtProperty& property) {
				static_cast<VariantProperty&>(property).setValue(mEntity->isDynamicsEnabled());
			},
			// Applier
			[this](const QtProperty& property) {
				mEntity->setDynamicsEnabled(static_cast<const VariantProperty&>(property).value.toBool());
			}
		);
	}

	update();

	emit modelReset(this);
}

void EntityPropertiesModel::onDestroy(sim::Entity* entity)
{
	mEntity = nullptr;
}
