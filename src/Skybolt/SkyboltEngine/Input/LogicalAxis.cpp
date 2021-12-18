/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#include "LogicalAxis.h"
#include "InputPlatform.h"
#include <SkyboltCommon/Math/MathUtility.h>
#include <boost/foreach.hpp>

using namespace skybolt;

KeyAxis::KeyAxis(const InputDevicePtr& device, int decreaseCode, int increaseCode, float rate, float restorationRate, float _min, float _max, float center) :
	mDevice(device),
	mRate(rate),
	mRestorationRate(restorationRate),
	mValue(center),
	mDecreaseCode(decreaseCode),
	mIncreaseCode(increaseCode),
	mMin(_min),
	mMax(_max),
	mCenter(center)
{
}

void KeyAxis::update(float dt)
{
	float input = (float)(-1 * mDevice->isButtonPressed(mDecreaseCode) + 1 * mDevice->isButtonPressed(mIncreaseCode));
	if (input != 0)
	{
		mValue += dt * mRate * input;
	}
	else // restore to neutral position
	{
		float delta = mCenter - mValue;
		if (delta > 0.0f)
			delta = std::min(mRestorationRate * dt, delta);
		else if (delta < 0.0f)
			delta = std::max(-mRestorationRate * dt, delta);
		mValue += delta;
	}

	mValue = skybolt::math::clamp(mValue, mMin, mMax);
}

FloatAxis::FloatAxis(const InputDevicePtr& device, size_t axisIndex, float minValue, float maxValue) :
	mDevice(device),
	mAxisIndex(axisIndex),
	mMinValue(minValue),
	mMaxValue(maxValue)
{
	assert(mDevice);
	assert(mAxisIndex < mDevice->getAxisCount());
}

float FloatAxis::getState() const
{
	return mMinValue + mDevice->getAxisState(mAxisIndex) * (mMaxValue - mMinValue);
}
