/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#include "Model.h"
#include "SkyboltVis/Camera.h"
#include "SkyboltVis/RenderContext.h"
#include "SkyboltVis/VisibilityCategory.h"
#include <assert.h>

using namespace skybolt::vis;

Model::Model(const ModelConfig &config) :
	mNode(config.node)
{
	assert(mNode);

	mTransform->setNodeMask(vis::VisibilityCategory::defaultCategories | vis::VisibilityCategory::shadowCaster);
	mTransform->addChild(mNode);

	mModelMatrix = new osg::Uniform("modelMatrix", osg::Matrixf());
	mTransform->getOrCreateStateSet()->addUniform(mModelMatrix);

	mModelViewMatrix = new osg::Uniform("modelViewMatrix", osg::Matrixf());
	mTransform->getOrCreateStateSet()->addUniform(mModelViewMatrix);
}

Model::~Model()
{
	mTransform->removeChild(mNode);
}

void Model::setMaxRenderDistance(float distance)
{
	// TODO: Implement
}

void Model::setVisibilityCategoryMask(uint32_t mask)
{
	mTransform->setNodeMask(mask);
}

void Model::setVisible(bool visible)
{
	if (visible != mVisible)
	{
		mVisible = visible;
		if (mVisible)
		{
			mTransform->addChild(mNode);
		}
		else
		{
			mTransform->removeChild(mNode);
		}
	}
}

void Model::updatePreRender(const CameraRenderContext& context)
{
	// Disable atmospheric shading if atmospheric density is too low because it causes rendering artifacts,
	// and atmospheric scattering is not very visible at high altitude.
	bool inAtmosphere = context.atmosphericDensity > 0.3;

	mTransform->getOrCreateStateSet()->setDefine("ENABLE_ATMOSPHERE", inAtmosphere);
	mModelMatrix->set(mTransform->getWorldMatrices().front());
	mModelViewMatrix->set(mTransform->getWorldMatrices().front() * context.camera.getViewMatrix());
}
