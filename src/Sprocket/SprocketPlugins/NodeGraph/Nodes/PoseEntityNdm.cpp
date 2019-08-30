/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "PoseEntityNdm.h"
#include "NodeContext.h"
#include <SkyboltSim/Components/DynamicBodyComponent.h>
#include <SkyboltSim/Entity.h>
#include <SkyboltSim/Spatial/Geocentric.h>
#include <SkyboltSim/Spatial/Orientation.h>
#include <SkyboltCommon/Math/MathUtility.h>

using namespace skybolt::sim;

PoseEntityNdm::PoseEntityNdm(NodeContext* context) :
	mContext(context)
{
	NodeDefPtr def = std::make_shared<NodeDef>();
	def->name = Name();
	def->inputs = { { EntityNodeData::typeId(), "entity" },{ PositionNodeData::typeId(), "position" },{OrientationNodeData::typeId(), "orientation" } };;
	setNodeDef(def);
}

void PoseEntityNdm::eval(const NodeDataVector& inputs) const
{
	Entity* entity = getDataOrDefault<EntityNodeData>(inputs[0].get());

	if (entity)
	{
		if (PositionNodeData* positionData = dynamic_cast<PositionNodeData*>(inputs[1].get()))
		{
			GeocentricPosition geocentricPosition = toGeocentric(*positionData->data);
			setPosition(*entity, geocentricPosition.position);

			if (OrientationNodeData* oriData = dynamic_cast<OrientationNodeData*>(inputs[2].get()))
			{
				LatLonAlt lla = toLatLonAlt(*positionData->data).position;

				GeocentricOrientation geocentricOrientation = toGeocentric(*oriData->data, toLatLon(lla));
				setOrientation(*entity, geocentricOrientation.orientation);
			}
		}
	}
}
