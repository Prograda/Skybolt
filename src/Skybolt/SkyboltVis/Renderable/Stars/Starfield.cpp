/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#include "SkyboltVis/Renderable/Stars/Starfield.h"
#include "SkyboltVis/Renderable/Stars/BrightStarCatelog.h"
#include "SkyboltVis/Camera.h"
#include "SkyboltVis/OsgGeometryFactory.h"
#include "SkyboltVis/OsgStateSetHelpers.h"
#include "SkyboltVis/RenderBinHelpers.h"
#include "SkyboltVis/RenderContext.h"
#include <SkyboltCommon/Math/MathUtility.h>

#include <osg/Geode>
#include <osg/ShapeDrawable>
#include <osg/Depth>
#include <osg/TextureBuffer>

//#define WIREFRAME
#ifdef WIREFRAME
#include <osg/PolygonMode>
#endif

using namespace skybolt::vis;

Starfield::Starfield(const StarfieldConfig& config)
{
	// Create geo
	osg::Geometry* geometry = createQuad(BoundingBox2f(osg::Vec2f(-0.5, -0.5), osg::Vec2f(0.5, 0.5)), QuadUpDirectionY);
	geometry->getPrimitiveSet(0)->setNumInstances(BrightStarCatelog::stars.size());
	geometry->setCullingActive(false);

	mGeode = new osg::Geode();
	mGeode->setCullingActive(false);

	mGeode->addDrawable(geometry);
	mTransform->addChild(mGeode);

	// Setup state
	osg::StateSet* ss = mGeode->getOrCreateStateSet();
	ss->setAttributeAndModes(config.program, osg::StateAttribute::ON);
	ss->setMode(GL_CULL_FACE, osg::StateAttribute::OFF);

	osg::Depth* depth = new osg::Depth;
	depth->setWriteMask(false);
	ss->setAttributeAndModes(depth, osg::StateAttribute::ON);

	mBrightnessUniform = new osg::Uniform("brightness", 1.0f);
	ss->addUniform(mBrightnessUniform);

	setRenderBin(*ss, RenderBinId::Stars);

	// Create texture to store per-instance parameters
	osg::ref_ptr<osg::Image> paramsImage = new osg::Image;
	paramsImage->allocateImage(BrightStarCatelog::stars.size(), 1, 1, GL_RGBA, GL_FLOAT);

	osg::Vec4f* ptr = (osg::Vec4f*)paramsImage->data();
	for (const BrightStarCatelog::Star& star : BrightStarCatelog::stars)
	{
		float cosElevation = cos(star.elevation);
		*ptr = osg::Vec4f(sin(star.azimuth*24) * cosElevation, cos(star.azimuth * 24) * cosElevation, sin(star.elevation), star.magnitude);
		++ptr;
	}

	osg::TextureBuffer* paramsTexture = new osg::TextureBuffer;
	paramsTexture->setImage(paramsImage);
	paramsTexture->setInternalFormat(GL_RGBA32F_ARB);
	ss->setTextureAttributeAndModes(0, paramsTexture, osg::StateAttribute::ON);

	ss->addUniform(createUniformSamplerTbo("paramsSampler", 0));
}

Starfield::~Starfield()
{
	mTransform->removeChild(mGeode);
}

void Starfield::updatePreRender(const RenderContext& context)
{
	setPosition(context.camera.getPosition());

	float skyBrightness = skybolt::math::clamp(context.atmosphericDensity * context.lightDirection.z() * -4.0f, 0.0f, 1.0f);
	float brightness = 1.0f - skyBrightness;
	mBrightnessUniform->set(brightness);
}
