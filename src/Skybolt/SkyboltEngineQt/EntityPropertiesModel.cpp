#include "EntityPropertiesModel.h"
#include <SkyboltCommon/MapUtility.h>
#include <SkyboltReflect/Reflection.h>
#include <SkyboltWidgets/Property/QtPropertyReflection.h>
#include <SkyboltSim/Components/NameComponent.h>
#include <SkyboltSim/Entity.h>

#include <boost/scope_exit.hpp>

using namespace skybolt;

EntityPropertiesModel::EntityPropertiesModel(refl::TypeRegistry* typeRegistry, const ReflTypePropertyFactoryMapPtr& factoryMap, sim::Entity* entity) :
	mTypeRegistry(typeRegistry),
	mReflTypePropertyFactoryMap(factoryMap),
	mEntity(nullptr)
{
	assert(mTypeRegistry);
	assert(mReflTypePropertyFactoryMap);

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
				ReflInstanceGetter getter = [this, component] { return refl::makeRefInstance(*mTypeRegistry, component.get()); };

				refl::Instance instance = refl::makeRefInstance(*mTypeRegistry, component.get());
				addReflPropertiesToModel(*mTypeRegistry, *this, toValuesVector(refl::getProperties(instance)), getter, *mReflTypePropertyFactoryMap);
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
					mEntity->setDynamicsEnabled(property.value().toBool());
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
	mEntity = nullptr;
}
