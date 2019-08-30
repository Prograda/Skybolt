/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "JoystickNdm.h"
#include "NodeContext.h"

using namespace skybolt;

JoystickNdm::JoystickNdm(NodeContext* context) :
	mContext(context)
{
	assert(mContext);

	NodeDefPtr def = std::make_shared<NodeDef>();
	def->name = Name();
	def->inputs = { { IntNodeData::typeId(), "index" } };
	def->outputs = {
		{ DoubleNodeData::typeId(), "a1" },
		{ DoubleNodeData::typeId(), "a2" },
		{ DoubleNodeData::typeId(), "a3" },
		{ DoubleNodeData::typeId(), "a4" }
	};
	setNodeDef(def);

	mConnections.push_back(context->timeSource->timeChanged.connect([this](double time) {SimpleNdm::eval(); }));
}

std::shared_ptr<QtNodes::NodeData> JoystickNdm::eval(const NodeDataVector& inputs, int outputIndex) const
{
	InputDevices joysticks = mContext->inputPlatform->getInputDevicesOfType(InputDeviceTypeJoystick);

	int i = getDataOrDefault<IntNodeData>(inputs[0].get());

	double value = 0;
	if (i < (int)joysticks.size())
	{
		const InputDevice& device = *joysticks[i];
		if (outputIndex < device.getAxisCount())
		{
			value = (double)device.getAxisState(outputIndex);
		}
	}
	return toNodeData<DoubleNodeData>(outputIndex, value);
}
