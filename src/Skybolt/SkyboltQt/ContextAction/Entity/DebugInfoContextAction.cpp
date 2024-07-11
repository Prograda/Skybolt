/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "DebugInfoContextAction.h"
#include <SkyboltSim/Spatial/Position.h>
#include <SkyboltCommon/Math/QuadTree.h>

#include <QMessageBox>
#include <osg/Vec2>

using namespace skybolt;
using namespace skybolt::sim;

bool DebugInfoContextAction::handles(const Entity& entity) const
{
	return getPosition(entity).has_value();
}

void DebugInfoContextAction::execute(Entity& entity) const
{
	int level = 14;
	LatLonAlt position = toLatLonAlt(GeocentricPosition(*getPosition(entity))).position;
	auto key = getKeyAtLevelIntersectingLonLatPoint<osg::Vec2>(level, osg::Vec2(position.lon, position.lat));

	std::string info;
	info += "Terrain tile: level: " + std::to_string(level) + ", x: " + std::to_string(key.x) + ", y: " + std::to_string(key.y) + "\n";

	QMessageBox::about(nullptr, "Debug Info", QString::fromStdString(info));
}
