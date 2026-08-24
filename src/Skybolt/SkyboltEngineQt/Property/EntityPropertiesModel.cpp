/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "EntityPropertiesModel.h"
#include <SkyboltCommon/MapUtility.h>
#include <SkyboltReflect/Reflection.h>
#include <SkyboltWidgets/Property/QtPropertyReflection.h>
#include <SkyboltSim/Components/NameComponent.h>
#include <SkyboltSim/Entity.h>

#include <boost/scope_exit.hpp>

using namespace skybolt;

EntityPropertiesModel::EntityPropertiesModel(refl::TypeRegistry* typeRegistry, const ReflValueTranslatorMapPtr& factoryMap, sim::Entity* entity) :
	mTypeRegistry(typeRegistry),
	mReflValueTranslatorMap(factoryMap),
	mEntity(nullptr)
{
	assert(mTypeRegistry);
	assert(mReflValueTranslatorMap);

	try
	{
		setEntity(entity);
	}
	// If exception was thrown from constructor, the destructor won't be called.
	// Remove the listener here to ensure constructor is cleaned up.
	catch (const std::exception& e)
	{
	if (mEntity)
		mEntity->removeListener(this);

		throw e;
	}
}

EntityPropertiesModel::~EntityPropertiesModel()
{
	if (mEntity)
		mEntity->removeListener(this);
}

void EntityPropertiesModel::setEntity(sim::Entity* entity)
{
	clearProperties();

	if (mEntity)
		mEntity->removeListener(this);

	mEntity = entity;

	if (mEntity)
	{
		mEntity->addListener(this);

		QtPropertyPtr nameProperty = createQtProperty(QLatin1String("name"), QString());
		nameProperty->setEnabled(false);
		addProperty(nameProperty, [this](QtValue& value, const PropertiesModel::QtValueUpdaterContext& context) {
			if (mEntity)
			{
				value.setValue(QString::fromStdString(getName(*mEntity)));
			}
		});

		for (const sim::ComponentPtr& component : mEntity->getComponents())
		{
			ReflInstanceGetter getter = [this, component] { return refl::makeRefInstance(*mTypeRegistry, component.get()); };

			refl::Instance instance = refl::makeRefInstance(*mTypeRegistry, component.get());
			addReflPropertiesToModel(*mTypeRegistry, *this, toValuesVector(refl::getProperties(instance)), getter, *mReflValueTranslatorMap);
		}

		addProperty(createQtProperty("dynamicsEnabled", false),
			// Updater
			[this](QtValue& value, const PropertiesModel::QtValueUpdaterContext& context) {
				if (mEntity)
				{
					value.setValue(mEntity->isDynamicsEnabled());
				}
			},
			// Applier
			[this](const QtValue& value, const PropertiesModel::QtValueApplierContext& context) {
				if (mEntity)
				{
					mEntity->setDynamicsEnabled(value.value().toBool());
				}
			},
			// Section name
			"Dynamics"
		);
	}

	update();

	emit modelReset(this);
}

void EntityPropertiesModel::onDestroy(sim::Entity* entity)
{
	setEntity(nullptr);
}
