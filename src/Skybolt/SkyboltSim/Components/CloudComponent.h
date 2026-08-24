/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltSim/Component.h"
#include "SkyboltSim/SkyboltSimFwd.h"

namespace skybolt::sim {

struct CloudLayer
{
	double bottomAltitude;
	double topAltitude;
	double density; //!< In range [0, inf)
	double coverageFraction; //!< Fraction of sky covered by this cloud layer, in range [0, 1].
	std::string type;
};

SKYBOLT_REFLECT(CloudLayer)
{
	registry.type<CloudLayer>("CloudLayer")
		.property("bottomAltitude", &CloudLayer::bottomAltitude)
		.property("topAltitude", &CloudLayer::topAltitude)
		.property("density", &CloudLayer::density)
		.property("coverageFraction", &CloudLayer::coverageFraction)
		.property("type", &CloudLayer::type);
};

struct CloudComponent : public skybolt::sim::Component
{
public:
	std::vector<CloudLayer> layers;
};


SKYBOLT_REFLECT(CloudComponent)
{
	registry.type<CloudComponent>("CloudsComponent")
		.superType<Component>()
		.property("layers", &CloudComponent::layers);
};

} // namespace skybolt::sim