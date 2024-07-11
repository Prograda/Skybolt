/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <SkyboltQt/Viewport/VisEntityAttachables.h>
#include <SkyboltSim/Entity.h>
#include <osg/Group>
#include <osg/MatrixTransform>

namespace skybolt {

osg::ref_ptr<osg::StateSet> createVisIconStateSet(const osg::ref_ptr<osg::Program>& program);

osg::ref_ptr<osg::Geode> createVisIconGeode(float radius);

} // namespace skybolt