/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "ContextAction.h"
#include <SkyboltSim/SkyboltSimFwd.h>
#include <SkyboltSim/SimMath.h>
#include <SkyboltSim/Spatial/LatLon.h>
#include <optional>

class QWidget;

struct ActionContext
{
	skybolt::sim::Entity* entity = nullptr; //!< Can be null
	std::optional<skybolt::sim::Vector3> point;
	QWidget* widget = nullptr; //!< Widget in which the action was performed
};

using DefaultContextAction = ContextAction<ActionContext>;
using DefaultContextActionPtr = std::shared_ptr<DefaultContextAction>;
