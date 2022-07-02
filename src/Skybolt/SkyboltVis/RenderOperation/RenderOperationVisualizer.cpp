/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "RenderOperationVisualizer.h"

#include "OsgStateSetHelpers.h"
#include "RenderOperationSequence.h"
#include "Renderable/ScreenQuad.h"

#include <osg/Camera>
#include <osg/Program>
#include <osg/Texture2D>

#include <assert.h>

namespace skybolt {
namespace vis {

RenderOperationVisualizer::RenderOperationVisualizer(const osg::ref_ptr<RenderOperation>& renderOperation, const osg::ref_ptr<osg::Program>& program) :
	mRenderOperation(renderOperation),
	mProgram(program),
	mCamera(new osg::Camera)
{
	assert(mRenderOperation);
	assert(mProgram);

	mCamera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
	mCamera->setClearMask(0);
	addChild(mCamera);
}

static osg::StateSet* createTexturedQuadStateSet(osg::ref_ptr<osg::Program> program, osg::ref_ptr<osg::Texture> texture)
{
	{
		// FIXME: Shadow texture visualizations do not display correctly if shadow comparison is enabled
		// because OpenGL can't read raw depth values from textures with shadow comparison enabled.
		// There might be a way to fix this by binding a second texture unit to the same underlying texture
		// with shadow comparison disabled.
		//texture->setShadowComparison(false);
	}

	osg::StateSet* stateSet = new osg::StateSet();
	stateSet->setAttributeAndModes(program, osg::StateAttribute::ON);
	stateSet->setTextureAttribute(0, texture, osg::StateAttribute::ON);
	stateSet->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
	stateSet->setMode(GL_CULL_FACE, osg::StateAttribute::OFF);
	
	stateSet->addUniform(createUniformSampler2d("tex", 0));

	return stateSet;
}

void RenderOperationVisualizer::updatePreRender(const RenderContext& renderContext)
{
	std::vector<osg::ref_ptr<osg::Texture>> textures = mRenderOperation->getOutputTextures();

	if (textures != mPrevTextures)
	{
		mPrevTextures = textures;

		constexpr int rowCount = 6;

		mCamera->removeChildren(0, mCamera->getNumChildren());
		mScreenQuads.clear();

		int x = 0;
		int y = 0;

		for (const auto& texture : textures)
		{
			if (texture)
			{
				float width = 1.0f / rowCount;
				float gap = width * 0.1f;

				osg::Vec2f size(width - gap, width - gap);
				osg::Vec2f pos(x * width + gap * 0.5, 1.0 - (size.y() + y * width + gap * 0.5));
				BoundingBox2f box(pos, pos + size);
				auto quad = std::make_shared<ScreenQuad>(createTexturedQuadStateSet(mProgram, texture), box);
				mCamera->addChild(quad->_getNode());
				mScreenQuads.push_back(quad);

				++x;
				if (x >= rowCount)
				{
					x = 0;
					++y;
				}
			}
		}
	}
}

} // namespace vis
} // namespace skybolt
