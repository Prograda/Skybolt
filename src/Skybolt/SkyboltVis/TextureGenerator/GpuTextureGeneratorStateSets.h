/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <osg/StateSet>
#include <osg/Texture2D>

namespace skybolt {
namespace vis {

osg::StateSet* createVectorDisplacementToNormalMapStateSet(osg::ref_ptr<osg::Program> program, osg::Texture2D* heightTexture, const osg::Vec2f& textureWorldSize);
osg::StateSet* createWaveFoamMaskGeneratorStateSet(osg::ref_ptr<osg::Program> program, osg::Texture2D* heightTexture, osg::Texture2D* prevOutputTexture, const osg::Vec2f& textureWorldSize, float lambda);

} // namespace vis
} // namespace skybolt
