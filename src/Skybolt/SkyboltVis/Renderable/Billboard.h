/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#pragma once

#include "SkyboltVis/DefaultRootNode.h"
#include <osg/Billboard>

namespace skybolt {
namespace vis {

//! Billboard normal is +X and up direction is -Z
class Billboard : public DefaultRootNode
{
public:
	Billboard(const osg::ref_ptr<osg::StateSet>& stateSet, float width, float height);
	~Billboard() override;

	void setOrientation(const osg::Quat &orientation) override {}; // has no effect

	void setUpDirection(const osg::Vec3f& dir);

protected:
	void updatePreRender(const RenderContext& context);

private:
	osg::Vec3f mUpDirection;
};

} // namespace vis
} // namespace skybolt
