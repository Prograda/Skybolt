/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <boost/signals2.hpp>
#include <tuple>

namespace skybolt {

struct TimeRange
{
	TimeRange(double start, double end) : start(start), end(end) {}
	bool operator==(const TimeRange& rhs) const
	{
		return std::make_tuple(start, end) == std::make_tuple(rhs.start, rhs.end);
	}

	bool operator!=(const TimeRange& rhs) const
	{
		return std::make_tuple(start, end) != std::make_tuple(rhs.start, rhs.end);
	}

	double start;
	double end;
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

	double getTime() const { return mTime; }
	void setTime(double time);

	const TimeRange& getRange() const { return mRange; }
	void setRange(const TimeRange& range);

	State getState() const { return mState; }
	void setState(const State& state);

	void advanceTime(double dt);

	boost::signals2::signal<void(const State&)> stateChanged;
	boost::signals2::signal<void(double)> timeChanged;
	boost::signals2::signal<void(const TimeRange&)> rangeChanged;

private:
	double mTime;
	TimeRange mRange;
	State mState;
};

} // namespace skybolt