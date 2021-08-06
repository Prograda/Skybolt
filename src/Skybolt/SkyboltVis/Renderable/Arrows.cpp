/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#include "Arrows.h"
#include "SkyboltVis/OsgGeometryHelpers.h"

#include <osg/Geode>
#include <osg/Geometry>

using namespace skybolt::vis;

Arrows::Arrows(const Params& params) :
	mGeometry(nullptr)
{
	mGeode = new osg::Geode();
	mGeode->getOrCreateStateSet()->setAttribute(params.program);
	mSwitch->addChild(mGeode);
}

Arrows::~Arrows()
{
	mSwitch->removeChild(mGeode);
}

void getOrthonormalBasis(const osg::Vec3f &normal, osg::Vec3f &tangent, osg::Vec3f &binormal)
{
	float d = normal * osg::Vec3f(0, 1, 0);
	if (d > -0.95f && d < 0.95f)
		binormal = normal ^ osg::Vec3f(0, 1, 0);
	else
		binormal = normal ^ osg::Vec3f(0, 0, 1);
	binormal.normalize();
	tangent = binormal ^ normal;
}

const float arrowHeadNormalScale = 0.1f;
osg::Vec2f arrowHeadTangentOffsets[] = {
	osg::Vec2f(-1, 0),
	osg::Vec2f(1, 0),
	osg::Vec2f(0, -1),
	osg::Vec2f(0, 1)
};

void Arrows::setSegments(const std::vector<Vec3Segment>& segments)
{
	if (mGeometry)
	{
		mGeode->removeChildren(0, 1);
		mGeometry = nullptr;
	}

	if (!segments.empty())
	{
		osg::Vec3Array* points = new osg::Vec3Array();
		points->reserve(segments.size() * 2);
		for (const Vec3Segment& segment : segments)
		{
			points->push_back(segment.start);
			points->push_back(segment.end);

			osg::Vec3f normal = segment.end - segment.start;
			float arrowHeadNormalLength = normal.normalize() * arrowHeadNormalScale;
			const float arrowHeadTangentLength = arrowHeadNormalLength * 0.5f;

			osg::Vec3f tangent, bitangent;
			getOrthonormalBasis(normal, tangent, bitangent);
			
			for (int i = 0; i < 4; ++i)
			{
				points->push_back(segment.end);
				points->push_back(
					segment.end
					- normal * arrowHeadNormalLength
					+ tangent * arrowHeadTangentOffsets[i].x() * arrowHeadTangentLength
					+ bitangent * arrowHeadTangentOffsets[i].y() * arrowHeadTangentLength
				);
			}
		}

		// Create new geometry
		mGeometry = new osg::Geometry;
		configureDrawable(*mGeometry);
		mGeode->addDrawable(mGeometry);

		// Add points to geometry
		osg::ref_ptr<osg::Vec4Array> color = new osg::Vec4Array;
		color->push_back(osg::Vec4(1.0, 0.0, 0.0, 1.0));
		mGeometry->setColorArray(color.get());
		mGeometry->setColorBinding(osg::Geometry::BIND_OVERALL);

		mGeometry->setVertexArray(points);
		mGeometry->getPrimitiveSetList().clear();
		mGeometry->addPrimitiveSet(new osg::DrawArrays(GL_LINES, 0, points->size()));
	}
}
