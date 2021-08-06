/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#include "Polyline.h"
#include "OsgGeometryHelpers.h"

#include <osg/Geode>
#include <osg/Geometry>

using namespace skybolt::vis;

Polyline::Polyline(const Params& params) :
	mGeometry(nullptr)
{
	mGeode = new osg::Geode();
	mGeode->getOrCreateStateSet()->setAttribute(params.program);
	mSwitch->addChild(mGeode);
}

Polyline::~Polyline()
{
	mSwitch->removeChild(mGeode);
}

void Polyline::setPoints(const osg::ref_ptr<osg::Vec3Array>& points)
{
	if (mGeometry)
	{
		mGeode->removeChildren(0, 1);
		mGeometry = nullptr;
	}

	if (points && !points->empty())
	{
		// Create new geometry
		mGeometry = new osg::Geometry;
		configureDrawable(*mGeometry);
		mGeode->addDrawable(mGeometry);

		// Add points to geometry
		osg::ref_ptr<osg::Vec4Array> color = new osg::Vec4Array;
		color->push_back(osg::Vec4(0.0, 1.0, 0.0, 1.0));
		mGeometry->setColorArray(color.get());
		mGeometry->setColorBinding(osg::Geometry::BIND_OVERALL);

		mGeometry->setVertexArray(points);
		mGeometry->getPrimitiveSetList().clear();
		mGeometry->addPrimitiveSet(new osg::DrawArrays(GL_LINE_STRIP, 0, points->size()));
	}
}
