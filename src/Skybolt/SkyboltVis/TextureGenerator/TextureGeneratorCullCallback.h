/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltVis/SkyboltVisFwd.h"
#include <osgUtil/CullVisitor>

namespace skybolt {
namespace vis {

//! Culls the subgraph *after* the first render per GraphicsContext.
//! This is useful for generating a texture only once in each GraphicsContext.
class TextureGeneratorCullCallback : public osg::NodeCallback
{
public:
	void operator()(osg::Node* node, osg::NodeVisitor* nv) override;

	//! Reset the culling, such that the texture will be regenerated on the next render,
	//! after which culling will resume.
	void resetCulling() { mRenderedForContexts.clear(); }

private:
	std::set<osg::GraphicsContext*> mRenderedForContexts;
};

} // namespace skybolt
} // namespace vis