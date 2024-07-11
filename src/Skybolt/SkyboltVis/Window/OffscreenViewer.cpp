/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "OffscreenViewer.h"

osg::ref_ptr<osg::GraphicsContext> createOffscreenContext(int width, int height)
{
	osg::ref_ptr<osg::GraphicsContext::Traits> traits(new osg::GraphicsContext::Traits);
	traits->x = 0;
	traits->y = 0;
	traits->width = width;
	traits->height = height;
	traits->red = 8;
	traits->green = 8;
	traits->blue = 8;
	traits->alpha = 8;
	traits->depth = 24;
	traits->windowDecoration = false;
	traits->pbuffer = true;
	traits->doubleBuffer = true;
	traits->sharedContext = 0x0;
	traits->samples = 16;

	return osg::GraphicsContext::createGraphicsContext(traits.get());
}

osg::ref_ptr<osgViewer::Viewer> createOffscreenViewer(int width, int height)
{
	osg::ref_ptr<osg::GraphicsContext> context = createOffscreenContext(width, height);
	if (!context)
	{
		throw std::runtime_error("Could not create offscreen OpenGL graphics context");
	}

	osg::ref_ptr<osgViewer::Viewer> viewer = new osgViewer::Viewer;
	viewer->setThreadingModel(osgViewer::Viewer::SingleThreaded);
	viewer->getCamera()->setGraphicsContext(context);
	viewer->getCamera()->setRenderTargetImplementation(osg::Camera::PIXEL_BUFFER);
	viewer->getCamera()->setViewport(new osg::Viewport(0, 0, width, height));
	viewer->realize();

	osg::State* state = context->getState();
	state->setUseModelViewAndProjectionUniforms(true);
	state->setUseVertexAttributeAliasing(true);

	return viewer;
}
