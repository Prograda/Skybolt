/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "VisEntityIcons.h"
#include "VisIcon.h"
#include <SkyboltVis/VisibilityCategory.h>
#include <SkyboltVis/Shader/ShaderProgramRegistry.h>
#include <osg/Geode>

namespace skybolt {

using namespace sim;

VisEntityIcons::VisEntityIcons(const vis::ShaderPrograms& programs, const osg::ref_ptr<osg::Texture>& iconTexture)
{
	setNodeMask(~vis::VisibilityCategory::shadowCaster);
	osg::ref_ptr<osg::StateSet> ss = createVisIconStateSet(programs.getRequiredProgram("hudGeometry"));
	ss->setTextureAttribute(0, iconTexture, osg::StateAttribute::ON);
	setStateSet(ss);
}

osg::ref_ptr<osg::Node> VisEntityIcons::createNode(sim::Entity* entity) const
{
	return createVisIconGeode(0.05f);
}

} // namespace skybolt