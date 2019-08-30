/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "GeocentricOrientationNdm.h"
#include "SkyboltSim/SimMath.h"
#include "SkyboltSim/Spatial/Orientation.h"

#include <SkyboltCommon/Math/MathUtility.h>

using namespace skybolt;

GeocentricOrientationNdm::GeocentricOrientationNdm(NodeContext* context)
{
	NodeDefPtr def = std::make_shared<NodeDef>();
	def->name = Name();
	def->inputs = {
		{ DoubleNodeData::typeId(), "yaw" },
		{ DoubleNodeData::typeId(), "pitch" },
		{ DoubleNodeData::typeId(), "roll" },
		{ AngleUnitNodeData::typeId(), "inputAngleUnits" }
	};
	def->outputs = { { OrientationNodeData::typeId(), "Ori" } };
	setNodeDef(def);
}

std::shared_ptr<QtNodes::NodeData> GeocentricOrientationNdm::eval(const NodeDataVector& inputs, int outputIndex) const
{
	sim::Vector3 v(
		getDataOrDefault<DoubleNodeData>(inputs[2].get()),
		getDataOrDefault<DoubleNodeData>(inputs[1].get()),
		getDataOrDefault<DoubleNodeData>(inputs[0].get())
	);

	AngleUnit unit = getDataOrDefault<AngleUnitNodeData>(inputs[3].get());
	if (unit == AngleUnitDegrees)
	{
		v *= math::degToRadD();
	}

	return toNodeData<OrientationNodeData>(outputIndex, std::make_shared<sim::GeocentricOrientation>(math::quatFromEuler(v)));
}
