/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltSim/Component.h"
#include "SkyboltSim/SkyboltSimFwd.h"

namespace skybolt::sim {

struct CloudComponent : public Component
{
public:
	bool cloudsVisible = true;
	std::optional<float> cloudCoverageFraction;
};

SKYBOLT_REFLECT_BEGIN(CloudComponent)
{
	registry.type<CloudComponent>("CloudComponent")
		.superType<Component>()
		.property("cloudsVisible", &CloudComponent::cloudsVisible)
		.property("cloudCoverageFraction", &CloudComponent::cloudCoverageFraction);
}
SKYBOLT_REFLECT_END

} // namespace skybolt::sim