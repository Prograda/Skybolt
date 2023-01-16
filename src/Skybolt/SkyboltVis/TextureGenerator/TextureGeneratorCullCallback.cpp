/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "TextureGeneratorCullCallback.h"

namespace skybolt {
namespace vis {

void TextureGeneratorCullCallback::operator()(osg::Node* node, osg::NodeVisitor* nv)
{
	osg::GraphicsContext* context = nv->asCullVisitor()->getCurrentCamera()->getGraphicsContext();
	if (mRenderedForContexts.find(context) == mRenderedForContexts.end())
	{
		mRenderedForContexts.insert(context);
		traverse(node, nv);
	}
}

} // namespace skybolt
} // namespace vis