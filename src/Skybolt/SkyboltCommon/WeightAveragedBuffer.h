/* Copyright Matthew Reid
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

protected:
	virtual float calcWeight(int index) const = 0;

protected:
	std::deque<float> mBuffer;
	unsigned int mSize;
};

class UniformAveragedBuffer : public WeightAveragedBuffer
{
public:
	UniformAveragedBuffer(unsigned int size) : WeightAveragedBuffer(size) {}

	float calcWeight(int index) const override { return 1; }
};

} // namespace skybolt