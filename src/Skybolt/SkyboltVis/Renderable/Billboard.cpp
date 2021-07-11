/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#include "Billboard.h"
#include "SkyboltVis/Camera.h"
#include "SkyboltVis/OsgGeometryFactory.h"
#include "SkyboltVis/MatrixHelpers.h"
#include "SkyboltVis/RenderContext.h"

using namespace skybolt::vis;

Billboard::Billboard(const osg::ref_ptr<osg::StateSet>& stateSet, float width, float height) :
	mUpDirection(osg::Vec3f(0, 0, -1))
{
	float halfWidth = width * 0.5f;
	float halfHeight = height * 0.5f;

	osg::ref_ptr<osg::Geometry> geometry = createQuadWithUvs(BoundingBox2f(osg::Vec2f(-halfWidth, -halfHeight), osg::Vec2f(halfWidth, halfHeight)), QuadUpDirectionY);
	geometry->setCullingActive(false); // TODO: compute bounds
	geometry->setStateSet(stateSet);

	mTransform->addChild(geometry);
}

Billboard::~Billboard()
{
}

void Billboard::updatePreRender(const RenderContext& context)
{
	osg::Vec3f N = context.camera.getPosition() - getPosition();
	N.normalize();

	osg::Vec3 upDirProj = mUpDirection - N * (mUpDirection * N);
	upDirProj.normalize();

	osg::Vec3f T = upDirProj ^ N;
	T.normalize();
	osg::Vec3f B = N ^ T;

	osg::Matrix m = makeMatrixFromTbn(T, B, N);

	DefaultRootNode::setOrientation(m.getRotate());
}

void Billboard::setUpDirection(const osg::Vec3f& dir)
{
	mUpDirection = dir;
}
