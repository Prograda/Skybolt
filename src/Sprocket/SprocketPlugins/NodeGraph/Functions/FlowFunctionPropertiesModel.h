/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "NodeGraphFwd.h"
#include <SkyboltEngine/SkyboltEngineFwd.h>
#include <Sprocket/PropertyEditor.h>

class FlowFunctionPropertiesModel : public PropertiesModel
{
public:
	FlowFunctionPropertiesModel(FlowFunction* flowFunction);

private:
	void recordAdded(QtProperty* property, int index, const TableRecord& record);
	void recordMoved(QtProperty* property, int oldIndex, int newIndex);
	void recordRemoved(QtProperty* property, int index);

private:
	FlowFunction* mFlowFunction;
	TablePropertyPtr mInputsProperty;
	TablePropertyPtr mOutputsProperty;
};
