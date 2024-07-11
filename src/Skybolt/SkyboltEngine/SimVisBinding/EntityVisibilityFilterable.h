/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SimVisBinding.h"
#include <SkyboltSim/World.h>

#include <osg/Switch>
#include <osg/MatrixTransform>

namespace skybolt {

using EntityVisibilityPredicate = std::function<bool(const sim::Entity&)>;
using EntityVisibilityPredicateSetter = std::function<void(EntityVisibilityPredicate)>;

 //! Visibility of entities is controlled by a predicate.
class EntityVisibilityFilterable
{
public:

	virtual void setEntityVisibilityPredicate(EntityVisibilityPredicate predicate) = 0;

	static bool visibilityOff(const sim::Entity&) { return false; }
	static bool visibilityOn(const sim::Entity&) {return true; }
};

} // namespace skybolt