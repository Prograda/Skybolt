/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "Plot3dNdm.h"
#include "NodeContext.h"
#include <SkyboltEngine/EntityFactory.h>
#include <SkyboltEngine/SimVisBinding/PolylineVisBinding.h>
#include <SkyboltSim/Entity.h>
#include <SkyboltSim/Spatial/Position.h>
#include <SkyboltCommon/Math/MathUtility.h>

using namespace skybolt;

Plot3dNdm::Plot3dNdm(NodeContext* context) :
	mFactory(context->entityFactory)
{
	mPolyline = mFactory->createEntity("Polyline");

	NodeDefPtr def = std::make_shared<NodeDef>();
	def->name = Name();
	def->inputs = {
		{ PositionVectorNodeData::typeId(), "position array" }
	};
	setNodeDef(def);
}

void Plot3dNdm::eval(const NodeDataVector& inputs) const
{
	const PositionVector& positions = getDataOrDefault<PositionVectorNodeData>(inputs[0].get());
	PolylineVisBinding* binding = static_cast<PolylineVisBinding*>(mPolyline->getFirstComponent<SimVisBindingsComponent>()->bindings.front().get());

	PositionsPtr points(new Positions(positions));
	binding->setPoints(points);
}
