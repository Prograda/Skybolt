/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "EntityVisibilityFilterable.h"
#include "SimVisBinding.h"
#include <SkyboltVis/SkyboltVisFwd.h>

namespace skybolt {

class ForcesVisBinding : public SimVisBinding, public EntityVisibilityFilterable
{
public:
	ForcesVisBinding(sim::World* world, const vis::ArrowsPtr& arrows);

public:
	// SimVisBinding interface
	void syncVis(const GeocentricToNedConverter& converter) override;

	void setEntityVisibilityPredicate(EntityVisibilityPredicate predicate) override
	{
		mEntityVisibilityPredicate = predicate;
	}

private:
	sim::World* mWorld;
	vis::ArrowsPtr mArrows;
	EntityVisibilityPredicate mEntityVisibilityPredicate;
};

std::shared_ptr<ForcesVisBinding> createForcesVisBinding(sim::World* world, const vis::ScenePtr& scene, const vis::ShaderPrograms& programs);

} // namespace skybolt