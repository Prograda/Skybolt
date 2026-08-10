/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltVis/SkyboltVisFwd.h"

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
		std::string albedoTextureFilename;
	};

	struct Roof
	{
		std::string albedoTextureFilename;
	};

	std::vector<Facade> facades;
	std::vector<Roof> roofs;
};

BuildingTypesPtr createBuildingTypesFromJson(const nlohmann::json& j);

} // namespace vis
} // namespace skybolt
