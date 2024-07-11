/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltVis/SkyboltVisFwd.h"
#include <osg/Group>
#include <osg/Texture>
#include <memory>

namespace skybolt {
namespace vis {

class TextureGeneratorCameraFactory
{
public:
	TextureGeneratorCameraFactory();
	~TextureGeneratorCameraFactory();

	osg::ref_ptr<osg::Camera> createCamera(std::vector<osg::ref_ptr<osg::Texture>> outputTextures, bool clear = true) const;
	osg::ref_ptr<osg::Camera> createCameraWithQuad(std::vector<osg::ref_ptr<osg::Texture>> outputTextures, const osg::ref_ptr<osg::StateSet>& stateSet, bool clear = true) const;
private:
	std::unique_ptr<ScreenQuad> mQuad;
};

} // namespace skybolt
} // namespace vis