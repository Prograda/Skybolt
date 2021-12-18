/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltEngine/SkyboltEngineFwd.h"
#include <osg/Vec2f>

namespace skybolt {

class LogicalAxis
{
public:
    virtual ~LogicalAxis() {}
	virtual void update(float dt) = 0;
	virtual void resetState() {};

	virtual float getState() const = 0;
};

class FloatAxis : public LogicalAxis
{
public:
	FloatAxis(const InputDevicePtr& device, size_t axisIndex, float minValue = 0.0f, float maxValue = 1.0f);
	void update(float dt) override {};

	float getState() const override;

private:
	const InputDevicePtr mDevice;
	const size_t mAxisIndex;
	const float mMinValue;
	const float mMaxValue;
};


class KeyAxis : public LogicalAxis
{
public:
	KeyAxis(const InputDevicePtr& device, int decreaseCode, int increaseCode, float rate, float restorationRate = 0, float _min = 0, float _max = 1, float _center = 0);
	void update(float dt) override;
	void resetState() override { mValue = mCenter; }

	float getState() const override {return mValue;}

private:
	InputDevicePtr mDevice;
	float mRate;
	float mRestorationRate;
	float mValue;
	int mDecreaseCode;
	int mIncreaseCode;
	float mMin;
	float mMax;
	float mCenter;
};

} // namespace skybolt