/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "BruentonAtmosphereRenderOperation.h"
#include "Scene.h"
#include "SkyboltVis/Renderable/Planet/Planet.h"

#include <assert.h>

namespace skybolt {
namespace vis {

BruentonAtmosphereRenderOperation::BruentonAtmosphereRenderOperation(const ScenePtr& scene) :
	mScene(scene)
{
	assert(mScene);
}

void BruentonAtmosphereRenderOperation::updatePreRender(const RenderContext& renderContext)
{
	removeChildren(0, getNumChildren());
	if (auto planet = mScene->getPrimaryPlanet(); planet)
	{
		if (auto atmosphere = planet->getAtmosphere(); atmosphere)
		{
			addChild(atmosphere);
			mScene->getStateSet()->merge(*atmosphere->getStateSet());
		}
	}
}

std::vector<osg::ref_ptr<osg::Texture>> BruentonAtmosphereRenderOperation::getOutputTextures() const
{
	if (auto planet = mScene->getPrimaryPlanet(); planet)
	{
		if (auto atmosphere = planet->getAtmosphere(); atmosphere)
		{
			return { atmosphere->getTransmittanceTexture(), atmosphere->getScatteringTexture(), atmosphere->getIrradianceTexture() };
		}
	}
	return {};
}

} // namespace vis
} // namespace skybolt