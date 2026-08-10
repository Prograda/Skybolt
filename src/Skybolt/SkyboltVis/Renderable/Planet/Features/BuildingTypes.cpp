/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "BuildingTypes.h"

namespace skybolt {
namespace vis {

BuildingTypesPtr createBuildingTypesFromJson(const nlohmann::json& j)
{
	auto types = std::make_shared<BuildingTypes>();

	auto facades = j.at("facades");

	for (const auto& item : facades.items())
	{
		const auto& jsonFacade = item.value();

		BuildingTypes::Facade facade;
		facade.buildingLevelsInTexture = jsonFacade.at("storiesInTexture");
		facade.horizontalSectionsInTexture = jsonFacade.at("horizontalSectionsInTexture");
		facade.albedoTextureFilename = jsonFacade.at("albedoTexture");

		types->facades.push_back(facade);
	}

	auto roofs = j.at("roofs");

	for (const auto& item : roofs.items())
	{
		const auto& jsonRoof = item.value();

		BuildingTypes::Roof roof;
		roof.albedoTextureFilename = jsonRoof.at("albedoTexture");
		types->roofs.push_back(roof);
	}

	return types;
}

} // namespace vis
} // namespace skybolt
