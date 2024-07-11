/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#include "CameraRelativeBillboard.h"
#include "SkyboltVis/Camera.h"
#include "SkyboltVis/OsgGeometryFactory.h"
#include "SkyboltVis/OsgGeometryHelpers.h"
#include "SkyboltVis/MatrixHelpers.h"
#include "SkyboltVis/RenderContext.h"

using namespace skybolt::vis;

CameraRelativeBillboard::CameraRelativeBillboard(const osg::ref_ptr<osg::StateSet>& stateSet, float width, float height, float distance) :
	mDistance(distance)
{
	float halfWidth = width * 0.5f;
	float halfHeight = height * 0.5f;

	osg::ref_ptr<osg::Geometry> geometry = createQuadWithUvs(BoundingBox2f(osg::Vec2f(-halfWidth, -halfHeight), osg::Vec2f(halfWidth, halfHeight)), QuadUpDirectionNegZ);
	geometry->setComputeBoundingBoxCallback(createFixedBoundingBoxCallback(osg::BoundingBox())); // TODO: set bounds
	geometry->setStateSet(stateSet);

	mTransform->addChild(geometry);
}

void CameraRelativeBillboard::updatePreRender(const CameraRenderContext& context)
{
	osg::Vec3f pos = context.camera.getPosition() + getOrientation() * osg::Vec3f(mDistance, 0, 0);
	DefaultRootNode::setPosition(pos);
}
