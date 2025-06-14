/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "ScenarioObjectPath.h"
#include "SkyboltSim/Component.h"

namespace skybolt {

struct ScenarioMetadataComponent : public sim::Component
{
	bool serializable = true; //!< True if the entity should be loaded and saved

	enum class LifetimePolicy
	{
		User, //!< The entity is created/deleted by the user from the UI.
		Procedural //! The entity is create/deleted procedurally and the user is not allowed to delete it from the UI.
	};
	//! True if the entity can be deleted by the user from the UI.
	
	LifetimePolicy lifetimePolicy = LifetimePolicy::User;

	ScenarioObjectPath directory; //!< Directory in the scenario hierarchy in which the entity resides
};

} // namespace skybolt