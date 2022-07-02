/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#pragma once

#include "SkyboltVis/SkyboltVisFwd.h"

#include <osg/Group>

namespace osg { class Texture; }

namespace skybolt {
namespace vis {

//! Represents a piece of work to be performed during render
class RenderOperation : public osg::Group
{
public:
	virtual void updatePreRender(const RenderContext& renderContext) {};

	virtual std::vector<osg::ref_ptr<osg::Texture>> getOutputTextures() const { return {}; };

	static osg::ref_ptr<RenderOperation> ofNode(const osg::ref_ptr<osg::Node>& node)
	{
		auto r = osg::ref_ptr<RenderOperation>(new RenderOperation);
		r->addChild(node);
		return r;
	}
};

} // namespace vis
} // namespace skybolt
