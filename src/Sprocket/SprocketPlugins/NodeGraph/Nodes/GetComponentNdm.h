/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SimpleNdm.h"

class GetComponentNdm : public SimpleNdm
{
public:
	GetComponentNdm(NodeContext* context);

	static QString Name() { return "GetComponent"; }

	virtual std::shared_ptr<QtNodes::NodeData> eval(const NodeDataVector& inputs, int outputIndex) const;
};
