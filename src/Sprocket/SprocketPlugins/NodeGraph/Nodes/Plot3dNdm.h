/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SimpleNdm.h"
#include <SkyboltEngine/SkyboltEngineFwd.h>

class Plot3dNdm : public SimpleNdm
{
public:
	Plot3dNdm(NodeContext* context);

	static QString Name() { return "Plot3d"; }

	void eval(const NodeDataVector& inputs) const override;

private:
	skybolt::EntityFactory* mFactory;
	mutable skybolt::sim::EntityPtr mPolyline; // TODO: come up with better solution than mutable. It's a hack to let eval() have side effects.
};
