/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "BuildingTypes.h"
#include "OsgImageHelpers.h"

namespace skybolt {
namespace vis {

osg::ref_ptr<osg::Texture2DArray> createTextureArray()
{
	osg::ref_ptr<osg::Texture2DArray> texture = new osg::Texture2DArray;
	texture->setInternalFormat(GL_SRGB8);
	texture->setSourceFormat(GL_RGB);
	texture->setWrap(osg::Texture::WRAP_S, osg::Texture::REPEAT);
	texture->setWrap(osg::Texture::WRAP_T, osg::Texture::REPEAT);
	texture->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR_MIPMAP_LINEAR);
	texture->setUseHardwareMipMapGeneration(true);
	return texture;
}

BuildingTypesPtr createBuildingTypesFromJson(const nlohmann::json& j)
{
	auto types = std::make_shared<BuildingTypes>();
	types->texture = createTextureArray();

	int i = 0;

	auto facades = j.at("facades");

	for (const auto& item : facades.items())
	{
		const auto& jsonFacade = item.value();

		osg::Image* image = readImageWithCorrectOrientation(jsonFacade.at("albedoTexture"));
		image->setInternalTextureFormat(toSrgbInternalFormat(image->getInternalTextureFormat()));
		types->texture->setImage(i, image);

		BuildingTypes::Facade facade;
		facade.buildingLevelsInTexture = jsonFacade.at("storiesInTexture");
		facade.horizontalSectionsInTexture = jsonFacade.at("horizontalSectionsInTexture");

		types->facades.push_back(facade);
		++i;
	}

	auto roofs = j.at("roofs");

	for (const auto& item : roofs.items())
	{
		const auto& jsonRoof = item.value();

		osg::Image* image = readImageWithCorrectOrientation(jsonRoof.at("albedoTexture"));
		image->setInternalTextureFormat(toSrgbInternalFormat(image->getInternalTextureFormat()));
		types->texture->setImage(i, image);

		++i;
	}
	types->roofCount = i - types->facades.size();

	return types;
}

} // namespace vis
} // namespace skybolt
