/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltQt/Viewport/VisEntityAttachables.h"
#include <SkyboltSim/Entity.h>
#include <osg/Group>
#include <osg/MatrixTransform>

namespace skybolt {

//! Displays an osg::Node, implemented by the derived class, at the position of each entity
class VisEntityIcons : public VisEntityAttachables
{
public:
	VisEntityIcons(const vis::ShaderPrograms& programs, const osg::ref_ptr<osg::Texture>& iconTexture);

protected:
	osg::ref_ptr<osg::Node> createNode(sim::Entity* entity) const override;
};

} // namespace skybolt