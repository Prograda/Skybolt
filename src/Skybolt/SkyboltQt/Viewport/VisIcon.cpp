/* Copyright 2012 - 2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0.If a copy of the MPL was not distributed with this
 * file, You can obtain one at https ://mozilla.org/MPL/2.0/. */


#include <Viewport/VisEntityAttachables.h>
#include <SkyboltVis/OsgGeometryFactory.h>
#include <osg/Depth>
#include <osg/Geode>
#include <osg/Group>
#include <osg/MatrixTransform>

namespace skybolt {

osg::ref_ptr<osg::StateSet> createVisIconStateSet(const osg::ref_ptr<osg::Program>& program)
{
	osg::ref_ptr<osg::StateSet> ss = new osg::StateSet();

	ss->setAttributeAndModes(program, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

	ss->setMode(GL_CULL_FACE, osg::StateAttribute::OFF);
	ss->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);
	ss->setMode(GL_BLEND, osg::StateAttribute::ON);
	ss->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

	ss->setDefine("USE_WORLD_SPACE_POSITION");
	ss->setDefine("USE_ASPECT_RATIO_CORRECTION");

	osg::Depth* depth = new osg::Depth;
	depth->setWriteMask(false);
	depth->setFunction(osg::Depth::ALWAYS);
	ss->setAttributeAndModes(depth, osg::StateAttribute::ON);

	return ss;
}

osg::ref_ptr<osg::Geode> createVisIconGeode(float radius)
{
	vis::BoundingBox2f bounds(osg::Vec2f(-radius, -radius), osg::Vec2f(radius, radius));
	osg::ref_ptr<osg::Geometry> geometry = vis::createQuadWithUvs(bounds, vis::QuadUpDirectionY);
	geometry->setCullingActive(false);

	osg::ref_ptr<osg::Geode> geode = new osg::Geode();
	geode->addDrawable(geometry);
	return geode;
}

} // namespace skybolt