/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "Sprocket/PropertyEditor.h"
#include <SkyboltEngine/SkyboltEngineFwd.h>
#include <SkyboltSim/Entity.h>
#include <SkyboltVis/SkyboltVisFwd.h>
#include <SkyboltCommon/Updatable.h>

class EntityPropertiesModel : public PropertiesModel, public skybolt::Updatable, public skybolt::sim::EntityListener
{
public:
	EntityPropertiesModel(skybolt::sim::Entity* entity = nullptr);
	~EntityPropertiesModel();

	void update() override;
	void setEntity(skybolt::sim::Entity* entity);

private:
	void onDestroy(skybolt::sim::Entity* entity) override;

private:
	void addLayerProperties(const skybolt::vis::PlanetTileSources& config, const std::string& name);

	typedef std::function<void(QtProperty&)> PropertyUpdater;
	typedef std::function<void(QtProperty&)> PropertyApplier;
	void addProperty(const QtPropertyPtr& property, PropertyUpdater updater);
	void addProperty(const QtPropertyPtr& property, PropertyUpdater updater, PropertyApplier applier);

private:
	skybolt::sim::Entity* mEntity;
	std::map<QtPropertyPtr, PropertyUpdater> mPropertyUpdaters;
	std::map<QtPropertyPtr, PropertyApplier> mPropertyAppliers;
};
