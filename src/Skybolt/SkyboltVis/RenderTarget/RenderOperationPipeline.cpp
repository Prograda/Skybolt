/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#include "RenderOperationPipeline.h"

#include <osg/Texture>

#include <assert.h>

namespace skybolt {
namespace vis {

RenderOperationPipeline::RenderOperationPipeline() :
	mRootNode(new osg::Group())
{
}

void RenderOperationPipeline::addOperation(const osg::ref_ptr<RenderOperation>& operation, int priority)
{
	mOperations.insert(std::make_pair(priority, operation));
	updateOsgGroup();
}

void RenderOperationPipeline::removeOperation(const osg::ref_ptr<RenderOperation>& operation)
{
	for (auto i = mOperations.begin(); i != mOperations.end(); ++i)
	{
		if (i->second == operation)
		{
			mOperations.erase(i);
			updateOsgGroup();
			return;
		}
	}
}

void RenderOperationPipeline::updatePreRender()
{
	for (const auto& [priority, operation] : mOperations)
	{
		operation->updatePreRender();
	}
}

void RenderOperationPipeline::updateOsgGroup()
{
	mRootNode->removeChildren(0, mRootNode->getNumChildren());
	
	for (const auto& [priority, operation]  : mOperations)
	{
		mRootNode->addChild(operation);
	}
}

} // namespace vis
} // namespace skybolt
