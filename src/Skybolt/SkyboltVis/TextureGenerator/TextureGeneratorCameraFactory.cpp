/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "TextureGeneratorCameraFactory.h"
#include <SkyboltVis/OsgStateSetHelpers.h>
#include "SkyboltVis/Renderable/ScreenQuad.h"

#include <osg/BlendEquation>
#include <osg/BlendFunc>
#include <osg/Camera>
#include <osg/Capability>

namespace skybolt {
namespace vis {

TextureGeneratorCameraFactory::TextureGeneratorCameraFactory()
{
	osg::ref_ptr<osg::StateSet> stateSet = new osg::StateSet();
	stateSet->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
	stateSet->setMode(GL_CULL_FACE, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
	mQuad = std::make_unique<ScreenQuad>(stateSet);
}

TextureGeneratorCameraFactory::~TextureGeneratorCameraFactory()
{
}

osg::ref_ptr<osg::Camera> TextureGeneratorCameraFactory::createCamera(std::vector<osg::ref_ptr<osg::Texture>> outputTextures, bool clear) const
{
	assert(!outputTextures.empty());
	osg::ref_ptr<osg::Camera> camera = new osg::Camera;
	camera->setClearMask(clear ? GL_COLOR_BUFFER_BIT : 0);
	camera->setViewport(0, 0, outputTextures[0]->getTextureWidth(), outputTextures[0]->getTextureHeight());
	camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
	camera->setRenderOrder(osg::Camera::PRE_RENDER);
	camera->setComputeNearFarMode(osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);
	camera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);
		
	// Ensure there is no depth buffer attached, otherwise we can't write to 3d texture layers.
	// OSG tries to automatically add a depth buffer for historical reasons, because
	// old drivers requred it. It's no longer needed. See https://groups.google.com/g/osg-users/c/_MksKA4Dmb4
	camera->detach(osg::Camera::DEPTH_BUFFER);

	// Set implicit color buffer attachment, which prevents OSG from automatically adding a
	// depth buffer if one is not attached.
	camera->setImplicitBufferAttachmentMask(osg::Camera::IMPLICIT_COLOR_BUFFER_ATTACHMENT);

	int face =  osg::Camera::FACE_CONTROLLED_BY_GEOMETRY_SHADER;
	for (int i = 0; i < outputTextures.size(); ++i)
	{
		camera->attach(osg::Camera::BufferComponent(osg::Camera::COLOR_BUFFER0 + i), outputTextures[i], 0, face);
	}
	return camera;
}

osg::ref_ptr<osg::Camera> TextureGeneratorCameraFactory::createCameraWithQuad(std::vector<osg::ref_ptr<osg::Texture>> outputTextures, const osg::ref_ptr<osg::StateSet>& stateSet, bool clear) const
{
	osg::ref_ptr<osg::Camera> camera = createCamera(outputTextures, clear);
	camera->addChild(mQuad->_getNode());

	// NOTE: we merge StateSet instead of setting, because Camera has it's own state already (e.g viewport) which we don't want to replace.
	camera->getOrCreateStateSet()->merge(*stateSet);

	return camera;
}

} // namespace skybolt
} // namespace vis