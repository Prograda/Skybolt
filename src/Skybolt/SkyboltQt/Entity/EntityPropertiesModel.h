/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltQt/Property/PropertyModel.h"
#include <SkyboltEngine/SkyboltEngineFwd.h>
#include <SkyboltReflection/SkyboltReflectionFwd.h>
#include <SkyboltSim/Entity.h>

class EntityPropertiesModel : public PropertiesModel, public skybolt::sim::EntityListener
{
public:
	EntityPropertiesModel(skybolt::refl::TypeRegistry* typeRegistry, skybolt::sim::Entity* entity = nullptr);
	~EntityPropertiesModel();

	void setEntity(skybolt::sim::Entity* entity);

private:
	void onDestroy(skybolt::sim::Entity* entity) override;

private:
	skybolt::refl::TypeRegistry* mTypeRegistry;
	skybolt::sim::Entity* mEntity;
	bool mCurrentlyUpdating = false;
};
