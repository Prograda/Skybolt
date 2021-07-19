/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "EntityPropertiesModel.h"
#include "Sprocket/QtTypeConversions.h"
#include <SkyboltCommon/Math/MathUtility.h>
#include <SkyboltEngine/VisObjectsComponent.h>
#include <SkyboltSim/Components/CameraComponent.h>
#include <SkyboltSim/Components/NameComponent.h>
#include <SkyboltSim/Entity.h>
#include <SkyboltVis/Renderable/Planet/Planet.h>
#include <SkyboltVis/Renderable/Planet/PlanetSurface.h>
#include <QVector3D>

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

void EntityPropertiesModel::update()
{
	if (!mEntity)
		return;

	for (const auto& entry : mPropertyUpdaters)
	{
		entry.second(*entry.first);
	}
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
		
		auto optionalPosition = getPosition(*mEntity);
		if (optionalPosition)
		{
			addProperty(createVariantProperty("position", QVector3D(0,0,0)), [this](QtProperty& property) {
				sim::Vector3 position = *getPosition(*mEntity);
				static_cast<VariantProperty&>(property).setValue(QVariant::fromValue(toQVector3D(position)));
			});

			addProperty(createVariantProperty("orientation", QVector3D(0, 0, 0)), [this](QtProperty& property) {
				glm::dvec3 euler = skybolt::math::eulerFromQuat(*getOrientation(*mEntity)) * skybolt::math::radToDegD();
				static_cast<VariantProperty&>(property).setValue(QVariant::fromValue(QVector3D(euler.x, euler.y, euler.z)));
			});
		}

		auto cameraComponent = mEntity->getFirstComponent<sim::CameraComponent>();
		if (cameraComponent)
		{
			addProperty(createVariantProperty("verticalFov", 0.0), [this, cameraComponent](QtProperty& property) {
				static_cast<VariantProperty&>(property).setValue(QVariant::fromValue(double(cameraComponent->getState().fovY * skybolt::math::radToDegF())));
			});
		}

		addProperty(createVariantProperty("dynamicsEnabled", false),
			// Updater
			[this](QtProperty& property) {
				static_cast<VariantProperty&>(property).setValue(mEntity->isDynamicsEnabled());
			},
			// Applier
			[this](QtProperty& property) {
				mEntity->setDynamicsEnabled(static_cast<VariantProperty&>(property).value.toBool());
			}
		);

		vis::Planet* planet = getFirstVisObject<vis::Planet>(*mEntity).get();
		if (planet)
		{
			addProperty(createVariantProperty("cloudsEnabled", true),
				// Updater
				[this, planet](QtProperty& property) {
					static_cast<VariantProperty&>(property).setValue(planet->getCloudsVisible());
				},
				// Applier
				[this, planet](QtProperty& property) {
					planet->setCloudsVisible(static_cast<VariantProperty&>(property).value.toBool());
				}
			);

			addProperty(createVariantProperty("cloudCoverageFraction", 0.0),
				// Updater
				[this, planet](QtProperty& property) {
					auto value = planet->getCloudCoverageFraction();
					static_cast<VariantProperty&>(property).setValue(value ? *value : -1.f);
				},
				// Applier
				[this, planet](QtProperty& property) {
					float value = static_cast<VariantProperty&>(property).value.toFloat();
					planet->setCloudCoverageFraction(value >= 0.f ? std::optional<float>(value) : std::nullopt);
				}
			);

			addProperty(createVariantProperty("waveHeight", 0.0),
				// Updater
				[this, planet](QtProperty& property) {
					static_cast<VariantProperty&>(property).setValue(planet->getWaveHeight());
				},
				// Applier
				[this, planet](QtProperty& property) {
					planet->setWaveHeight(static_cast<VariantProperty&>(property).value.toFloat());
				}
			);
		}
	}

	update();

	emit modelReset(this);
}

void EntityPropertiesModel::onDestroy(sim::Entity* entity)
{
	mEntity = nullptr;
}

void EntityPropertiesModel::addProperty(const QtPropertyPtr& property, PropertyUpdater updater)
{
	mProperties.push_back(property);
	property->setEnabled(false);
	mPropertyUpdaters[property] = updater;
}

void EntityPropertiesModel::addProperty(const QtPropertyPtr& property, PropertyUpdater updater, PropertyApplier applier)
{
	mProperties.push_back(property);
	mPropertyUpdaters[property] = updater;
	mPropertyAppliers[property] = applier;

	connect(property.get(), &QtProperty::valueChanged, this, [=]() {
		applier(*property);
	});
}
