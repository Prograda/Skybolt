/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "FunctionNdm.h"
#include "Functions/FlowFunction.h"

FunctionNdm::FunctionNdm(const FlowFunctionPtr& function) :
	mFunction(function)
{
	setNodeDef(function->getNodeDef());

	function->registerUser(this);
}

FunctionNdm::~FunctionNdm()
{
	mFunction->unregisterUser(this);
}

std::shared_ptr<QtNodes::NodeData> FunctionNdm::eval(const NodeDataVector& inputs, int outputIndex) const
{
	// TODO: don't re-evaluate for every output
	NodeDataPtrVector outputs = mFunction->eval(inputs);
	return (outputIndex < outputs.size()) ? outputs[outputIndex] : nullptr;
}
