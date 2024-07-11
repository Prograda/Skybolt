/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "ScreenQuad.h"
#include "OsgGeometryFactory.h"

#include <osg/Geometry>
#include <osg/Geode>
#include <osg/Projection>

namespace skybolt {
namespace vis {

ScreenQuad::ScreenQuad(osg::StateSet* stateSet, const BoundingBox2f& bounds)
{
	mTransform->addChild(createGeode(stateSet, bounds));
}

osg::ref_ptr<osg::Geode> ScreenQuad::createGeode(osg::StateSet* stateSet, const BoundingBox2f& bounds)
{
	osg::Geometry* quad = createQuadWithUvs(bounds, QuadUpDirectionY);

	quad->setStateSet(stateSet);
	quad->setCullingActive(false);

	osg::Geode* geode = new osg::Geode(); 
	geode->addDrawable(quad);
	return geode;
}

osg::ref_ptr<osg::Group> ScreenQuad::createNode(osg::StateSet* stateSet, const BoundingBox2f& bounds)
{
	osg::ref_ptr<osg::Group> group = new osg::Group();
	group->addChild(createGeode(stateSet, bounds));
	return group;
}

} // namespace vis
} // namespace skybolt
