/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltQt/ContextAction/ActionContext.h"
#include <SkyboltSim/Entity.h>

using EntityContextAction = ContextAction<skybolt::sim::Entity>;
using EntityContextActionPtr = std::shared_ptr<EntityContextAction>;

class EntityContextActionAdapter : public ContextAction<ActionContext>
{
public:
	EntityContextActionAdapter(const std::shared_ptr<EntityContextAction>& wrapped) : mWrapped(wrapped) {}

	std::string getName() const override { return mWrapped->getName(); }

	bool handles(const ActionContext& object) const override
	{
		if (object.entity)
		{
			return mWrapped->handles(*object.entity);
		}
		return false;
	}

	void execute(ActionContext& object) const override
	{
		if (object.entity)
		{
			mWrapped->execute(*object.entity);
		}
	}

private:
	EntityContextActionPtr mWrapped;
};

inline std::shared_ptr<EntityContextActionAdapter> adaptToDefaultAction(std::shared_ptr<EntityContextAction> action)
{
	return std::make_shared<EntityContextActionAdapter>(std::move(action));
}