/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "Chrono.h"

namespace skybolt {
namespace sim {

enum class UpdateStage
{
	Input,
	BeginStateUpdate,
	PreDynamicsSubStep,
	DynamicsSubStep,
	PostDynamicsSubStep,
	EndStateUpdate,
	Attachments,
	Output
};

class SimUpdatable
{
public:
	virtual ~SimUpdatable() = default;

	virtual void setSimTime(SecondsD newTime) {};
	virtual void advanceWallTime(SecondsD newTime, SecondsD dt) {};
	virtual void advanceSimTime(SecondsD newTime, SecondsD dt) {};
	virtual void update(UpdateStage stage) {};
};

/*! Helper macros for registering update handlers within a class definition. Usage example:

class MyClass : public SimUpdatable
{
	SKYBOLT_BEGIN_REGISTER_UPDATE_HANDLERS
		SKYBOLT_REGISTER_UPDATE_HANDLER(UpdateStage::Attachments, myFunc)
	SKYBOLT_END_REGISTER_UPDATE_HANDLERS

	void myFunc();
};
*/
#define SKYBOLT_BEGIN_REGISTER_UPDATE_HANDLERS \
	void update(skybolt::sim::UpdateStage stage) override { \
		switch (stage) {

#define SKYBOLT_REGISTER_UPDATE_HANDLER(stage, func) \
		case stage: func(); break;

#define SKYBOLT_END_REGISTER_UPDATE_HANDLERS }}

} // namespace sim
} // namespace skybolt