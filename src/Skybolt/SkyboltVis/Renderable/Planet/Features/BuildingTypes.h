/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltVis/SkyboltVisFwd.h"
#include <osg/Texture2DArray>

#include <vector>
#include <nlohmann/json.hpp>

namespace skybolt {
namespace vis {

struct BuildingTypes
{
	struct Facade
	{
		int buildingLevelsInTexture;
		int horizontalSectionsInTexture;
	};

	//! Texture array containing facades, then roofs
	osg::ref_ptr<osg::Texture2DArray> texture;

	std::vector<Facade> facades;
	int roofCount;
};

BuildingTypesPtr createBuildingTypesFromJson(const nlohmann::json& j);

} // namespace vis
} // namespace skybolt
