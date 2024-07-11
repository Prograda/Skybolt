/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "RenderEnvironmentMap.h"
#include "Scene.h"
#include "SkyboltVis/GlobalSamplerUnit.h"
#include "SkyboltVis/OsgStateSetHelpers.h"
#include "SkyboltVis/OsgTextureHelpers.h"
#include "SkyboltVis/Renderable/Planet/Planet.h"
#include "SkyboltVis/TextureGenerator/GpuTextureGenerator.h"

#include <assert.h>

namespace skybolt {
namespace vis {

static osg::StateSet* createSkyToEnvironmentMapStateSet(osg::ref_ptr<osg::Program> program)
{
	osg::StateSet* stateSet = new osg::StateSet();
	stateSet->setAttributeAndModes(program, osg::StateAttribute::ON);
	return stateSet;
}

static osg::ref_ptr<osg::Texture2D> createEnvironmentTexture(int width, int height)
{
	osg::ref_ptr<osg::Texture2D> texture = createRenderTexture(width, height);
	texture->setInternalFormat(GL_RGB16F_ARB);
	texture->setFilter(osg::Texture2D::FilterParameter::MIN_FILTER, osg::Texture2D::FilterMode::LINEAR_MIPMAP_LINEAR);
	texture->setFilter(osg::Texture2D::FilterParameter::MAG_FILTER, osg::Texture2D::FilterMode::LINEAR);
	texture->setWrap(osg::Texture::WRAP_S, osg::Texture::REPEAT);
	texture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);

	return texture;
}

RenderEnvironmentMap::RenderEnvironmentMap(const ScenePtr& scene, const osg::ref_ptr<osg::StateSet> viewportStateSet, const ShaderPrograms& programs) :
	mScene(scene)
{
	assert(mScene);

	osg::ref_ptr<osg::Texture2D> environmentTexture = createEnvironmentTexture(128, 128);
	mGenerator = new GpuTextureGenerator(environmentTexture, createSkyToEnvironmentMapStateSet(programs.getRequiredProgram("skyToEnvironmentMap")), /* generateMipMaps */ true);
	addChild(mGenerator);

	viewportStateSet->setTextureAttributeAndModes((int)GlobalSamplerUnit::EnvironmentProbe, environmentTexture, osg::StateAttribute::ON);
	viewportStateSet->addUniform(createUniformSampler2d("environmentSampler", (int)GlobalSamplerUnit::EnvironmentProbe));
}

void RenderEnvironmentMap::updatePreRender(const RenderContext& renderContext)
{
	// TODO: set map to black if there is no atmosphere
	if (auto planet = mScene->getPrimaryPlanet(); planet)
	{
		if (auto atmosphere = planet->getAtmosphere(); atmosphere)
		{
			mGenerator->requestRegenerate();
		}
	}
}

std::vector<osg::ref_ptr<osg::Texture>> RenderEnvironmentMap::getOutputTextures() const
{
	return mGenerator->getOutputTextures();
}

} // namespace vis
} // namespace skybolt