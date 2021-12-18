/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "EntityStateSequenceController.h"
#include "SkyboltEngine/Sequence/Interpolator/CubicBSplineInterpolator.h"
#include "SkyboltEngine/Sequence/Interpolator/LinearInterpolator.h"
#include <SkyboltSim/World.h>
#include <assert.h>

namespace skybolt {

constexpr bool linear = false;

std::shared_ptr<InterpolatorD> createPositionComponentInterpolator(const std::shared_ptr<EntityStateSequence>& sequence, int componentIndex)
{
	if (linear)
	{
		return std::make_shared<LinearInterpolatorD>(
			[sequence, componentIndex](int i) { return sequence->values[i].position[componentIndex]; });
	}
	else
	{
		return std::make_shared<CubicBSplineInterpolatorD>(
			[sequence, componentIndex] { return sequence->values.size(); },
			[sequence, componentIndex](int i) { return sequence->values[i].position[componentIndex]; },
			[sequence, componentIndex](int i) { return sequence->times[i]; });
	}
}

std::shared_ptr<InterpolatorD> createOrientationComponentInterpolator(const std::shared_ptr<EntityStateSequence>& sequence, int componentIndex)
{
	if (linear)
	{
		return std::make_shared<LinearInterpolatorD>(
			[sequence, componentIndex](int i) { return sequence->values[i].orientation[componentIndex]; });
	}
	else
	{
		return std::make_shared<CubicBSplineInterpolatorD>(
			[sequence, componentIndex] { return sequence->values.size(); },
			[sequence, componentIndex](int i) { return sequence->values[i].orientation[componentIndex]; },
			[sequence, componentIndex](int i) { return sequence->times[i]; });
	}
}

static glm::dquat toSameSign(const glm::dquat& self, const glm::dquat& reference)
{
	double dot = glm::dot(self, reference);
	return dot >= 0.0 ? self : -self;
}

// Needed because quaternions are ambigious, i.e -q == q, and we must ensure they're
// all consistent so as to interpolate the shorter distance
static void makeQuaternionSignConsistent(EntityStateSequence& sequence)
{
	if (!sequence.values.empty())
	{
		glm::dquat first = sequence.values.front().orientation;
		for (size_t i = 1; i < sequence.values.size(); ++i)
		{
			sequence.values[i].orientation = toSameSign(sequence.values[i].orientation, first);
		}
	}
}

EntityStateSequenceController::EntityStateSequenceController(const std::shared_ptr<EntityStateSequence>& sequence) :
	StateSequenceControllerT(sequence)
{
	makeQuaternionSignConsistent(*sequence);

	mPositionInterpolators = {
		createPositionComponentInterpolator(sequence, 0),
		createPositionComponentInterpolator(sequence, 1),
		createPositionComponentInterpolator(sequence, 2)
	};

	mOrientationInterpolators = {
		createOrientationComponentInterpolator(sequence, 0),
		createOrientationComponentInterpolator(sequence, 1),
		createOrientationComponentInterpolator(sequence, 2),
		createOrientationComponentInterpolator(sequence, 3)
	};

	auto sanitizeOrientation = [sequence](size_t index) {
		glm::dquat& ori = sequence->values[index].orientation;
		ori = toSameSign(ori, sequence->values.front().orientation);
	};

	mConnections.push_back(sequence->itemAdded.connect(sanitizeOrientation));
	mConnections.push_back(sequence->valueChanged.connect(sanitizeOrientation));
}

EntityStateSequenceController::~EntityStateSequenceController()
{
	if (mEntity)
	{
		mEntity->removeListener(this);
	}
}

SequenceStatePtr EntityStateSequenceController::getState() const
{
	if (mEntity)
	{
		auto position = sim::getPosition(*mEntity);
		auto orientation = sim::getOrientation(*mEntity);

		if (position && orientation)
		{
			auto state = std::make_shared<EntitySequenceState>();
			state->position = *position;
			state->orientation = *orientation;
			return state;
		}
	}

	return nullptr;
}

void EntityStateSequenceController::setEntity(sim::Entity* entity)
{
	if (mEntity)
	{
		mEntity->removeListener(this);
	}
	
	mEntity = entity;
	entityChanged(mEntity);
	
	if (entity)
	{
		entity->addListener(this);
	}
}

void EntityStateSequenceController::setStateT(const EntitySequenceState& state)
{
	if (mEntity)
	{
		setPosition(*mEntity, state.position);
		setOrientation(*mEntity, state.orientation);
	}
}

SequenceStatePtr EntityStateSequenceController::getStateAtInterpolationPoint(const math::InterpolationPoint& point) const
{
	glm::dvec3 position;
	for (int i = 0; i < (int)mPositionInterpolators.size(); ++i)
	{
		position[i] = mPositionInterpolators[i]->interpolate(point.bounds.first, point.bounds.last, point.weight);
	}

	glm::dquat orientation;
	for (int i = 0; i < (int)mOrientationInterpolators.size(); ++i)
	{
		orientation[i] = mOrientationInterpolators[i]->interpolate(point.bounds.first, point.bounds.last, point.weight);
	}

	auto state = std::make_shared<EntitySequenceState>();
	state->position = position;
	state->orientation = glm::normalize(orientation);
	return state;
}

SequenceStatePtr EntityStateSequenceController::getStateAtTime(double t) const
{
#ifdef SMOOTHING_TEST
	EntitySequenceState finalState;
	finalState.position = glm::dvec3(0);
	finalState.orientation = glm::dquat(0, 0, 0, 0);

	double halfWidth = 2;
	int sampleCount = 10;
	for (int i = 0; i < sampleCount; ++i)
	{
		double tSample = glm::mix(t - halfWidth, t + halfWidth, double(i) / double(sampleCount));
		boost::optional<math::InterpolationPoint> point = math::findInterpolationPoint(mSequence->times, tSample, /* extrapolate */ false);
		if (point)
		{
			const EntitySequenceState& state = static_cast<const EntitySequenceState&>(*getStateAtInterpolationPoint(*point));
			finalState.position += state.position;
			for (int j=0; j<4; ++j)
				finalState.orientation[j] += state.orientation[j];
		}
	}
	finalState.position /= sampleCount;
	finalState.orientation = glm::normalize(finalState.orientation);

	return std::make_shared<EntitySequenceState>(finalState);
#else
	return StateSequenceControllerT<EntitySequenceState>::getStateAtTime(t);
#endif
}

void EntityStateSequenceController::onDestroy(sim::Entity* entity)
{
	mEntity = nullptr;
	entityChanged(mEntity);
}

} // namespace skybolt