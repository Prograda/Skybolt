/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "GpuTextureGeneratorStateSets.h"
#include "SkyboltVis/OsgMathHelpers.h"
#include "SkyboltVis/OsgStateSetHelpers.h"
#include <SkyboltCommon/Math/MathUtility.h>

namespace skybolt {
namespace vis {

osg::StateSet* createVectorDisplacementToNormalMapStateSet(osg::ref_ptr<osg::Program> program, osg::Texture2D* heightTexture, const osg::Vec2f& textureWorldSize)
{
	osg::StateSet* stateSet = new osg::StateSet();
	stateSet->setAttributeAndModes(program, osg::StateAttribute::ON);
	stateSet->setTextureAttributeAndModes(0, heightTexture, osg::StateAttribute::ON);

	const osg::Image& image = *heightTexture->getImage(0);
	osg::Vec2f texelSize(1.0f / (float)image.s(), 1.0f / (float)image.t());

	stateSet->addUniform(new osg::Uniform("texelSizeInTextureSpace", texelSize));
	stateSet->addUniform(new osg::Uniform("textureSizeInWorldSpace", textureWorldSize));

	return stateSet;
}

osg::StateSet* createWaveFoamMaskGeneratorStateSet(osg::ref_ptr<osg::Program> program, osg::Texture2D* heightTexture, osg::Texture2D* prevOutputTexture, const osg::Vec2f& textureWorldSize, float lambda)
{
	osg::StateSet* stateSet = new osg::StateSet();
	stateSet->setAttributeAndModes(program, osg::StateAttribute::ON);
	stateSet->setTextureAttributeAndModes(0, heightTexture, osg::StateAttribute::ON);
	stateSet->setTextureAttributeAndModes(1, prevOutputTexture, osg::StateAttribute::ON);

	stateSet->addUniform(createUniformSampler2d("displacementSampler", 0));
	stateSet->addUniform(createUniformSampler2d("prevOutputSampler", 1));

	const osg::Image& image = *heightTexture->getImage(0);
	osg::Vec2f texelSizeInTextureSpace(1.0f / (float)image.s(), 1.0f / (float)image.t());
	osg::Vec2f texelSizeInWorldSpace = math::componentWiseMultiply(texelSizeInTextureSpace, textureWorldSize);

	stateSet->addUniform(new osg::Uniform("lambda", lambda));
	stateSet->addUniform(new osg::Uniform("texelSizeInTextureSpace", texelSizeInTextureSpace));
	stateSet->addUniform(new osg::Uniform("texelSizeInWorldSpace", texelSizeInWorldSpace));

	return stateSet;
}

} // namespace vis
} // namespace skybolt
