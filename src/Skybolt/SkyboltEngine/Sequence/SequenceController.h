/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltEngine/SkyboltEngineFwd.h"
#include <SkyboltSim/SkyboltSimFwd.h>
#include <SkyboltCommon/Math/InterpolateTableLinear.h>

#include <boost/signals2.hpp>
#include <algorithm>
#include <memory>
#include <optional>
#include <vector>

namespace skybolt {

struct SequenceState
{
	virtual ~SequenceState() {}
	virtual std::string toString() const = 0;
};

using SequenceStatePtr = std::shared_ptr<SequenceState>;


struct Sequence
{
	std::vector<double> times; //!< Times must be monotonically increasing

	boost::signals2::signal<void(const size_t&)> itemAdded;
	boost::signals2::signal<void(const size_t&)> valueChanged;
	boost::signals2::signal<void(const size_t&)> itemRemoved;

	std::optional<size_t> getIndexAtTime(double time) const
	{
		for (size_t i = 0; i < times.size(); ++i)
		{
			if (times[i] == time)
			{
				return i;
			}
			else if (times[i] > time)
			{
				break;
			}
		}
		return std::nullopt;
	}

	virtual void addItemAtIndex(const SequenceState& value, double time, size_t index) = 0;
	virtual void addItemAtTime(const SequenceState& value, double time) = 0;
	virtual void removeItemAtIndex(size_t index) = 0;
	virtual const SequenceState& getItemAtIndex(size_t index) = 0;

	virtual void setValueAtIndex(const SequenceState& value, size_t index) = 0;

	virtual void clear() = 0;
};

template <typename T> // T must derive from SequenceState 
struct StateSequenceT : public Sequence
{
	std::vector<T> values;

	void addItemAtIndex(const SequenceState& value, double time, size_t index) override
	{
		times.insert(times.begin() + index, time);
		values.insert(values.begin() + index, static_cast<const T&>(value));
		itemAdded(index);
	}

	void addItemAtTime(const SequenceState& value, double time) override
	{
		for (size_t i = 0; i < times.size(); ++i)
		{
			if (time < times[i])
			{
				addItemAtIndex(value, time, i);
				return;
			}
		}
		addItemAtIndex(value, time, times.size());
	}

	void removeItemAtIndex(size_t index) override
	{
		times.erase(times.begin() + index);
		values.erase(values.begin() + index);
		itemRemoved(index);
	}

	virtual const SequenceState& getItemAtIndex(size_t index) override
	{
		return values[index];
	}

	void setValueAtIndex(const SequenceState& value, size_t index) override
	{
		values[index] = static_cast<const T&>(value);
		valueChanged(index);
	}

	void clear() override
	{
		while (!times.empty())
		{
			removeItemAtIndex(0);
		}
	}
};

using SequencePtr = std::shared_ptr<Sequence>;

class SequenceController
{
public:
	virtual ~SequenceController() {}

	virtual void setTime(double t) = 0;
	virtual SequencePtr getSequence() const = 0;
};

class StateSequenceController : public SequenceController
{
public:
	virtual ~StateSequenceController() {}

	virtual SequenceStatePtr getStateAtTime(double t) const = 0; //!< Returns nullptr if no state available at time
	virtual SequenceStatePtr getState() const = 0;
	virtual void setState(const SequenceState& state) = 0;
};

template <class T>
class StateSequenceControllerT : public StateSequenceController
{
public:
	StateSequenceControllerT(const std::shared_ptr<StateSequenceT<T>>& sequence) :
		mSequence(sequence)
	{
		assert(mSequence);
	}

	SequencePtr getSequence() const override { return mSequence; }

	SequenceStatePtr getStateAtTime(double t) const override
	{
		std::optional<math::InterpolationPoint> point = math::findInterpolationPoint(mSequence->times, t, /* extrapolate */ false);
		if (point)
		{
			return getStateAtInterpolationPoint(*point);
		}
		return nullptr;
	}

	void setTime(double t) override
	{
		SequenceStatePtr state = getStateAtTime(t);
		if (state)
		{
			setState(*state);
		}
	}

	void setState(const SequenceState& state) override
	{
		const auto& derivedState = dynamic_cast<const T&>(state);
		setStateT(derivedState);
	}

protected:
	virtual void setStateT(const T& state) = 0;
	virtual SequenceStatePtr getStateAtInterpolationPoint(const math::InterpolationPoint& point) const = 0;

protected:
	std::shared_ptr<StateSequenceT<T>> mSequence;
};

} // namespace skybolt