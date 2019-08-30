/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#pragma once

#include "SimpleNdm.h"
#include <SkyboltSim/SkyboltSimFwd.h>

class PoseEntityNdm : public SimpleNdm
{
public:
	PoseEntityNdm(NodeContext* context);

	static QString Name() { return "PoseEntity"; }

	void eval(const NodeDataVector& inputs) const override;

private:
	NodeContext * mContext;
};
