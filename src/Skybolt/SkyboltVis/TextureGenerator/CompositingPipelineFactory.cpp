/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "CompositingPipelineFactory.h"
#include "TextureGeneratorCameraFactory.h"
#include <SkyboltVis/OsgStateSetHelpers.h>
#include "SkyboltVis/Renderable/ScreenQuad.h"

#include <osg/BlendEquation>
#include <osg/BlendFunc>
#include <osg/Camera>
#include <osg/Capability>

namespace skybolt {
namespace vis {

CompositingPipelineFactory::CompositingPipelineFactory() :
	mTextureGeneratorCameraFactory(std::make_unique<TextureGeneratorCameraFactory>())
{
}

CompositingPipelineFactory::~CompositingPipelineFactory()
{
}

osg::ref_ptr<osg::Group> CompositingPipelineFactory::createCompositingPipeline(const std::vector<CompositingStage>& stages)
{
	osg::ref_ptr<osg::Group> group = new osg::Group();

	for (const CompositingStage& stage : stages)
	{
		osg::ref_ptr<osg::StateSet> stateSet = new osg::StateSet();
		stateSet->setAttribute(stage.program);
		stateSet->setAttributeAndModes(new osg::BlendEquation(osg::BlendEquation::FUNC_ADD, osg::BlendEquation::FUNC_ADD));
		stateSet->setAttributeAndModes(new osg::BlendFunc(osg::BlendFunc::ONE, osg::BlendFunc::ONE, osg::BlendFunc::ONE, osg::BlendFunc::ONE));

		for (const auto& uniform : stage.uniforms)
		{
			stateSet->addUniform(uniform);
		}

		for (int i = 0; i < stage.inputs.size(); ++i)
		{
			auto input = stage.inputs[i];
			stateSet->setTextureAttributeAndModes(i, input.texture);
			stateSet->addUniform(createUniformSampler2d(input.samplerName, i));
		}

		for (int i = 0; i < stage.outputs.size(); ++i)
		{
			bool additive = (i < stage.additive.size()) && stage.additive[i];
			if (additive)
			{
				stateSet->setAttribute(new osg::Enablei(GL_BLEND, i));
			}
			else
			{
				stateSet->setAttribute(new osg::Disablei(GL_BLEND, i));
			}
		}

		auto camera = mTextureGeneratorCameraFactory->createCameraWithQuad(stage.outputs, stateSet, stage.clear);
		group->addChild(camera);
	}

	return group;
}

} // namespace skybolt
} // namespace vis