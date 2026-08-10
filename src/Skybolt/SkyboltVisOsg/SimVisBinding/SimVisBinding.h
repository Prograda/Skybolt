/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#pragma once

#include "SkyboltEngine/SkyboltEngineFwd.h"
#include <SkyboltSim/Component.h>
#include <SkyboltSim/SkyboltSimFwd.h>
#include "SkyboltVisOsg/SkyboltVisOsgFwd.h"

namespace skybolt {

class SimVisBinding
{
public:
	virtual ~SimVisBinding() {};

	virtual void syncVis(const GeocentricToNedConverter& converter) = 0;
};


class SimpleSimVisBinding : public SimVisBinding
{
public:
	//! Create binding with no vis objects. Call addVisObject() to add objects after construction.
	SimpleSimVisBinding(const sim::Entity* entity);

	//! Create binding with one vis object
	SimpleSimVisBinding(const sim::Entity* entity, const vis::RootNodePtr& visObject,
					const glm::dvec3& visPositionOffset = math::dvec3Zero(), const glm::dquat& visOrientationOffset = math::dquatIdentity());

	void addVisObject(const vis::RootNodePtr& visObject,
		const glm::dvec3& visPositionOffset = math::dvec3Zero(), const glm::dquat& visOrientationOffset = math::dquatIdentity());

	void syncVis(const GeocentricToNedConverter& converter) override;

protected:
	const sim::Entity* mEntity;

	struct VisItem
	{
		const vis::RootNodePtr object;
		glm::dvec3 positionOffset;
		glm::dquat orientationOffset;
	};

	std::vector<VisItem> mVisObjects;
};

struct SimVisBindingsComponent : public sim::Component
{
	std::vector<SimVisBindingPtr> bindings;
};

typedef std::shared_ptr<SimVisBindingsComponent> SimVisBindingsComponentPtr;

void syncVis(const sim::World& world, const GeocentricToNedConverter& converter);

} // namespace skybolt