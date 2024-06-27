/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "EntityPropertiesModel.h"
#include "SkyboltQt/Property/QtPropertyReflection.h"
#include <SkyboltCommon/MapUtility.h>
#include <SkyboltReflection/Reflection.h>
#include <SkyboltSim/Components/NameComponent.h>
#include <SkyboltSim/Entity.h>

#include <boost/scope_exit.hpp>

using namespace skybolt;

EntityPropertiesModel::EntityPropertiesModel(refl::TypeRegistry* typeRegistry, sim::Entity* entity) :
	mTypeRegistry(typeRegistry),
	mEntity(nullptr)
{
	assert(mTypeRegistry);
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

		QtPropertyPtr nameProperty = createQtProperty(QLatin1String("name"), QString());
		nameProperty->setEnabled(false);
		addProperty(nameProperty, [this](QtProperty& property) {
			if (mEntity)
			{
				property.setValue(QString::fromStdString(getName(*mEntity)));
			}
		});

		for (const sim::ComponentPtr& component : mEntity->getComponents())
		{
			if (refl::TypePtr type = mTypeRegistry->getMostDerivedType(*component); type)
			{
				ReflInstanceGetter getter = [this, component] { return refl::createNonOwningInstance(mTypeRegistry, component.get()); };
				addRttrPropertiesToModel(*mTypeRegistry, *this, toValuesVector(type->getProperties()), getter);
			}
		}

		addProperty(createQtProperty("dynamicsEnabled", false),
			// Updater
			[this](QtProperty& property) {
				if (mEntity)
				{
					property.setValue(mEntity->isDynamicsEnabled());
				}
			},
			// Applier
			[this](const QtProperty& property) {
				if (mEntity)
				{
					mEntity->setDynamicsEnabled(property.value.toBool());
				}
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
