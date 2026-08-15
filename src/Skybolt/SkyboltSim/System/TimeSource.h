/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltSim/Chrono.h"
#include <boost/signals2.hpp>
#include <tuple>

namespace skybolt::sim {

struct TimeRange
{
	TimeRange(SecondsD start, SecondsD end) : start(start), end(end) {}
	bool operator==(const TimeRange& rhs) const
	{
		return std::make_tuple(start, end) == std::make_tuple(rhs.start, rhs.end);
	}

	bool operator!=(const TimeRange& rhs) const
	{
		return std::make_tuple(start, end) != std::make_tuple(rhs.start, rhs.end);
	}

	SecondsD getDuration() const { return end - start; }

	SecondsD start;
	SecondsD end;
};

class TimeSource
{
public:
	TimeSource(const TimeRange& range);

	enum State
	{
		StatePlaying,
		StateStopped
	};

	SecondsD getTime() const { return mTime; }
	virtual void setTime(sim::SecondsD time);

	virtual void advanceTime(sim::SecondsD dt);

	virtual void advanceToTime(sim::SecondsD time);

	const TimeRange& getRange() const { return mRange; }
	void setRange(const TimeRange& range);

	State getState() const { return mState; }
	void setState(const State& state);

	boost::signals2::signal<void(const State&)> stateChanged;
	boost::signals2::signal<void(SecondsD oldTime, SecondsD newTime)> timeAboutToChange;
	boost::signals2::signal<void(SecondsD)> timeChanged;
	boost::signals2::signal<void(const TimeRange&)> rangeChanged;

protected:
	sim::SecondsD mTime;
	TimeRange mRange;
	State mState;
};

} // namespace skybolt::sim