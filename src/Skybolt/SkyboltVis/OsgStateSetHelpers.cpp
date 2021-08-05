/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "OsgStateSetHelpers.h"

#include <osg/BlendEquation>
#include <osg/BlendFunc>
#include <osg/Depth>
#include <osg/StateSet>

namespace skybolt {
namespace vis {

void makeStateSetTransparent(osg::StateSet& stateSet, TransparencyMode transparencyMode)
{
	stateSet.setAttributeAndModes(new osg::BlendEquation(osg::BlendEquation::FUNC_ADD, osg::BlendEquation::FUNC_ADD));

	osg::BlendFunc::BlendFuncMode sourceMode = (transparencyMode == TransparencyMode::PremultipliedAlpha) ? osg::BlendFunc::ONE : osg::BlendFunc::SRC_ALPHA;
	stateSet.setAttributeAndModes(new osg::BlendFunc(sourceMode, osg::BlendFunc::ONE_MINUS_SRC_ALPHA));

	stateSet.setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

	osg::Depth* depth = new osg::Depth;
	depth->setWriteMask(false);
	stateSet.setAttributeAndModes(depth, osg::StateAttribute::ON);
}

} // namespace vis
} // namespace skybolt
