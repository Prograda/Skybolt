/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#include "WeightAveragedBuffer.h"

namespace skybolt {

WeightAveragedBuffer::WeightAveragedBuffer(unsigned int size)
{
	setSize(size);
}

void WeightAveragedBuffer::addValue(float value)
{
	unsigned int bufferSize = (unsigned int)mBuffer.size();
	if (bufferSize == mSize)
	{
		mBuffer.pop_front();
		mBuffer.push_back(value);
	}
	else if (bufferSize < mSize)
	{
		mBuffer.push_back(value);
	}
	else //if (bufferSize > mSize)
	{
		mBuffer.pop_front();
	}
}

void WeightAveragedBuffer::clear()
{
	mBuffer.clear();
}

float WeightAveragedBuffer::getResult() const
{
	float result = 0;
	int bufferSize = (int)mBuffer.size();
	float invBufferSize = 1.0f / bufferSize;
	
	for (int i=0; i<bufferSize; i++)
		result += mBuffer[i] * (i+1) * invBufferSize;

	return result / ((bufferSize + 1)*0.5f);
}

} //namespace