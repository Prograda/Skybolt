/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "NodeGraphFwd.h"
#include "SimpleNdm.h"
#include "Functions/FlowFunctionRegistry.h"
#include <Sprocket/SprocketFwd.h>
#include <QLabel>

class MapSamplesNdm : public SimpleNdm
{
public:
	MapSamplesNdm(NodeContext* context);

	static QString Name() { return "MapSamples"; }

	std::shared_ptr<QtNodes::NodeData> eval(const NodeDataVector& inputs, int outputIndex) const override;
private:
	FlowFunctionRegistry* mFlowFunctionRegistry;
};
