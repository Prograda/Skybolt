/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#pragma once

#include <deque>

namespace skybolt {

class WeightAveragedBuffer
{
public:
	explicit WeightAveragedBuffer(unsigned int size);
	inline void setSize(unsigned int size) {mSize = size;}

	void addValue(float value);
	void clear();

	float getResult() const;

private:
	std::deque<float> mBuffer;
	unsigned int mSize;
};

} // namespace skybolt