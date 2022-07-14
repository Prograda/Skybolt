/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "CloudsRenderTexture.h"
#include "VolumeClouds.h"
#include "SkyboltVis/Camera.h"
#include "SkyboltVis/RenderContext.h"
#include "SkyboltVis/Scene.h"

namespace skybolt {
namespace vis {

osg::ref_ptr<RenderTexture> createCloudsRenderTexture(const ScenePtr& scene)
{
	RenderTextureConfig c;
	c.colorTextureFactories = { createScreenTextureFactory(GL_RGBA), createScreenTextureFactory(GL_R32F) };
	c.multisampleSampleCount = 0;
	c.clear = false;

	osg::ref_ptr<osg::Group> group = scene->getBucketGroup(Scene::Bucket::Clouds);

	osg::ref_ptr<RenderTexture> texture = new RenderTexture(c);
	texture->setScene(group);
	texture->setRelativeRect(RectF(0, 0, 0.25, 0.25));
	return texture;
}

} // namespace vis
} // namespace skybolt
