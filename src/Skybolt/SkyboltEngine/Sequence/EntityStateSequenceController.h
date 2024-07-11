/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SequenceController.h"
#include "SkyboltEngine/Sequence/Interpolator/Interpolator.h"
#include <SkyboltSim/Entity.h>
#include <SkyboltSim/SimMath.h>
#include <SkyboltCommon/Math/MathUtility.h>

namespace skybolt {

struct EntitySequenceState : public SequenceState
{
	sim::Vector3 position;
	sim::Quaternion orientation;

	std::string toString() const override
	{
		return "p: "
			+ math::toString(position)
			+ " q: "
			+ math::toString(orientation);
	}
};

using EntityStateSequence = StateSequenceT<EntitySequenceState>;

class EntityStateSequenceController : public StateSequenceControllerT<EntitySequenceState>, public sim::Entity::Listener
{
public:
	EntityStateSequenceController(const std::shared_ptr<EntityStateSequence>& sequence);
	~EntityStateSequenceController() override;

	void setEntity(sim::Entity* entity);
	sim::Entity* getEntity() const { return mEntity; }

	SequenceStatePtr getState() const override;

	void setStateT(const EntitySequenceState& state) override;
	SequenceStatePtr getStateAtInterpolationPoint(const math::InterpolationPoint& point) const override;

	SequenceStatePtr getStateAtTime(double t) const override;

	boost::signals2::signal<void(sim::Entity* entity)> entityChanged;

private:
	void onDestroy(sim::Entity* entity) override;

private:
	sim::Entity* mEntity = nullptr;
	std::vector<std::shared_ptr<InterpolatorD>> mPositionInterpolators;
	std::vector<std::shared_ptr<InterpolatorD>> mOrientationInterpolators;
	std::vector<boost::signals2::connection> mConnections;
};

} // namespace skybolt