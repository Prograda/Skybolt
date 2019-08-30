/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltVis/SkyboltVisFwd.h"
#include "SkyboltVis/Renderable/ScreenQuad.h"

#include <osg/Texture2D>

namespace skybolt {
namespace vis {

class GpuTextureGenerator : public osg::Group
{
public:
	GpuTextureGenerator(const osg::ref_ptr<osg::Texture2D>& texture, const osg::ref_ptr<osg::StateSet>& stateSet, bool generateMipMaps);
	~GpuTextureGenerator();

	void requestRender();

	const osg::ref_ptr<osg::Camera>& getCamera() const { return mCamera; }

private:
	osg::ref_ptr<osg::Camera> mCamera;
	std::unique_ptr<ScreenQuad> mQuad;
	bool mActive;

	friend class DrawCallback;
};

} // namespace vis
} // namespace skybolt
