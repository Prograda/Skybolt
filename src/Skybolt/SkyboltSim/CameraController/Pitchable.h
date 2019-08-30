/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

namespace skybolt {
namespace sim {

class Pitchable
{
public:
	~Pitchable() = default;
	virtual double getPitch() const { return mPitch; }
	virtual void setPitch(double pitch) { mPitch = pitch; }

protected:
	double mPitch = 0;
};

} // namespace sim
} // namespace skybolt