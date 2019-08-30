/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltSim/Component.h"
#include "SkyboltSim/Entity.h"

namespace skybolt {
namespace sim {

class ParentReferenceComponent : public Component, public sim::EntityListener
{
public:
	ParentReferenceComponent(sim::Entity* parent) : mParent(parent)
	{
		parent->addListener(this);
	}

	~ParentReferenceComponent()
	{
		if (mParent)
		{
			mParent->removeListener(this);
		}
	}

	sim::Entity* getParent() const { return mParent; }

private:
	void onDestroy(Entity* entity) override { mParent = nullptr; }

private:
	sim::Entity* mParent;
};

} // namespace sim
} // namespace skybolt