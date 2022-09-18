/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "WaterMaterialRenderOperation.h"
#include "Scene.h"
#include "SkyboltVis/Renderable/Planet/Planet.h"
#include "SkyboltVis/Renderable/Water/WaterMaterial.h"

#include <assert.h>

namespace skybolt {
namespace vis {

WaterMaterialRenderOperation::WaterMaterialRenderOperation(const ScenePtr& scene, const ShaderPrograms& programs) :
	mScene(scene)
{
	assert(mScene);
}

void WaterMaterialRenderOperation::updatePreRender(const RenderContext& renderContext)
{
	removeChildren(0, getNumChildren());
	if (auto planet = mScene->getPrimaryPlanet(); planet)
	{
		if (const auto& material = planet->getWaterMaterial(); material)
		{
			addChild(material);
		}
	}
}

std::vector<osg::ref_ptr<osg::Texture>> WaterMaterialRenderOperation::getOutputTextures() const
{
	if (auto planet = mScene->getPrimaryPlanet(); planet)
	{
		if (const auto& material = planet->getWaterMaterial(); material)
		{
			for (int i = 0; i < material->getCascadeCount(); ++i)
			{
				return { material->getHeightTexture(i), material->getNormalTexture(i), material->getFoamMaskTexture(i) };
			}
		}
	}
	return {};
}

} // namespace vis
} // namespace skybolt