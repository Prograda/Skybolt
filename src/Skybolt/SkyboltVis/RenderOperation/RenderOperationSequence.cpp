/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#include "RenderOperationSequence.h"

#include <osg/Texture>

#include <assert.h>

namespace skybolt {
namespace vis {

RenderOperationSequence::RenderOperationSequence() :
	mRootNode(new osg::Group())
{
}

void RenderOperationSequence::addOperation(const osg::ref_ptr<RenderOperation>& operation, int priority)
{
	mOperations.insert(std::make_pair(priority, operation));
	updateOsgGroup();
}

void RenderOperationSequence::removeOperation(const osg::ref_ptr<RenderOperation>& operation)
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

void RenderOperationSequence::updatePreRender(const RenderContext& context)
{
	for (const auto& [priority, operation] : mOperations)
	{
		operation->updatePreRender(context);
	}
}

void RenderOperationSequence::updateOsgGroup()
{
	mRootNode->removeChildren(0, mRootNode->getNumChildren());
	
	for (const auto& [priority, operation]  : mOperations)
	{
		mRootNode->addChild(operation);
	}
}

std::vector<osg::ref_ptr<osg::Texture>> RenderOperationSequence::getOutputTextures() const
{
	std::vector<osg::ref_ptr<osg::Texture>> textures;
	
	for (const auto& [priority, operation] : mOperations)
	{
		const auto& operationTextures = operation->getOutputTextures();
		textures.insert(textures.end(), operationTextures.begin(), operationTextures.end());
	}
	return textures;
}

} // namespace vis
} // namespace skybolt
