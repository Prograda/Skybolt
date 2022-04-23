/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#include "PlanetSky.h"
#include "SkyboltVis/Camera.h"
#include "SkyboltVis/OsgGeometryFactory.h"
#include <SkyboltVis/OsgGeometryHelpers.h>
#include "SkyboltVis/RenderBinHelpers.h"
#include "SkyboltVis/RenderContext.h"

#include <osg/BlendEquation>
#include <osg/BlendFunc>
#include <osg/Geode>
#include <osg/ShapeDrawable>
#include <osg/Depth>

//#define WIREFRAME
#ifdef WIREFRAME
#include <osg/PolygonMode>
#endif

using namespace skybolt::vis;

static osg::StateSet* createStateSet(const osg::ref_ptr<osg::Program>& program)
{
	osg::StateSet* stateSet = new osg::StateSet();

#ifdef WIREFRAME
	osg::PolygonMode * polygonMode = new osg::PolygonMode;
	polygonMode->setMode(osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::LINE);

	stateSet->setAttributeAndModes(polygonMode, osg::StateAttribute::OVERRIDE | osg::StateAttribute::ON);
#endif

	osg::Depth* depth = new osg::Depth;
	depth->setWriteMask(false);
	stateSet->setAttributeAndModes(depth, osg::StateAttribute::ON);
	stateSet->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);
	stateSet->setAttributeAndModes(new osg::BlendEquation(osg::BlendEquation::FUNC_ADD, osg::BlendEquation::FUNC_ADD));
	stateSet->setAttributeAndModes(new osg::BlendFunc(osg::BlendFunc::ONE, osg::BlendFunc::ONE));

	stateSet->setAttribute(program);
	setRenderBin(*stateSet, RenderBinId::Sky);

	return stateSet;
}

PlanetSky::PlanetSky(const PlanetSkyConfig& config) :
	radius(config.radius)
{
	osg::Geometry* geometry = createSphere(config.radius, 256, 128);
	vis::configureDrawable(*geometry);

	mGeode = new osg::Geode();
	mGeode->setCullingActive(false);

	mGeode->setStateSet(createStateSet(config.program));
	mGeode->addDrawable(geometry);
	mTransform->addChild(mGeode);
}

PlanetSky::~PlanetSky()
{
	mTransform->removeChild(mGeode);
}
