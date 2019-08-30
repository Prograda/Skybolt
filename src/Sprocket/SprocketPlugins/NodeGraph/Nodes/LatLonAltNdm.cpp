/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "LatLonAltNdm.h"
#include <SkyboltSim/Spatial/Position.h>
#include <SkyboltCommon/Math/MathUtility.h>

using namespace skybolt;

LatLonAltNdm::LatLonAltNdm(NodeContext* context)
{
	NodeDefPtr def = std::make_shared<NodeDef>();
	def->name = Name();
	def->inputs = {
		{ DoubleNodeData::typeId(), "latitude" },
		{ DoubleNodeData::typeId(), "longitude" },
		{ DoubleNodeData::typeId(), "altitude" },
		{ AngleUnitNodeData::typeId(), "inputAngleUnits" }
	};
	def->outputs = { { PositionNodeData::typeId(), "P" } };
	setNodeDef(def);
}

std::shared_ptr<QtNodes::NodeData> LatLonAltNdm::eval(const NodeDataVector& inputs, int outputIndex) const
{
	sim::LatLonAlt v(
		getDataOrDefault<DoubleNodeData>(inputs[0].get()),
		getDataOrDefault<DoubleNodeData>(inputs[1].get()),
		getDataOrDefault<DoubleNodeData>(inputs[2].get())
	);

	AngleUnit unit = getDataOrDefault<AngleUnitNodeData>(inputs[3].get());
	if (unit == AngleUnitDegrees)
	{
		v.lat *= math::degToRadD();
		v.lon *= math::degToRadD();
	}

	return toNodeData<PositionNodeData>(outputIndex, std::make_shared<sim::LatLonAltPosition>(v));
}
