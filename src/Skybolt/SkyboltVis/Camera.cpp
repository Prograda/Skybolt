/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#include "Camera.h"
#include "MatrixHelpers.h"
#include "RenderContext.h"
#include "VisibilityCategory.h"
#include "Window/Window.h"

#include <SkyboltCommon/Math/MathUtility.h>

#include <osg/CullFace>

using namespace skybolt::vis;

Camera::Camera(float aspectRatio) :
	mAspectRatio(aspectRatio),
	mFovY(0.5f),
	mNear(1),
	mFar(1e6),
	mCullMask(VisibilityCategory::primary)
{
	setPosition(osg::Vec3f());
	setOrientation(osg::Quat());
	updateProjectionMatrix();
}

Camera::~Camera()
{
}

void Camera::setPosition(const osg::Vec3d &position)
{
	mPosition = position;
	updateViewMatrix();
}

void Camera::setOrientation(const osg::Quat &orientaiton)
{
	mOrientation = orientaiton;
	updateViewMatrix();
}

void Camera::setNearClipDistance(float near)
{
	mNear = near;
	updateProjectionMatrix();
}

void Camera::setFarClipDistance(float far)
{
	mFar = far;
	updateProjectionMatrix();
}

void Camera::setFovY(float fovY)
{
	mFovY = fovY;
	updateProjectionMatrix();
}

void Camera::setAspectRatio(float aspectRatio)
{
	mAspectRatio = aspectRatio;
	updateProjectionMatrix();
}

void Camera::updateOsgCameraGeometry(osg::Camera& camera) const
{
	camera.setViewMatrix(mViewMatrix);
	camera.setProjectionMatrix(mProjectionMatrix);
	camera.setCullMask(mCullMask);
}

void Camera::updateViewMatrix()
{
	static osg::Quat orientationOffset = osg::Quat(skybolt::math::halfPiF(), osg::Vec3f(0,-1,0)) * osg::Quat(skybolt::math::halfPiF(), osg::Vec3f(-1,0,0));
	osg::Matrix m;
	m.setTrans(mPosition);
	m.setRotate(orientationOffset * mOrientation);

	mViewMatrix = osg::Matrix::inverse(m);
}

void Camera::updateProjectionMatrix()
{
	mProjectionMatrix = osg::Matrixd::perspective(mFovY * skybolt::math::radToDegF(), mAspectRatio, mNear, mFar);

	if (mObliqueClippingPlane)
	{
		// Translate the plane into view space
		osg::Plane plane = mul(*mObliqueClippingPlane, mViewMatrix);

		// Thanks to Eric Lenyel for posting this calculation 
		// at www.terathon.com
		osg::Matrix projMatrix = mProjectionMatrix;
		osg::Vec4f qVec;
		qVec.x() = (osg::signOrZero(plane.getNormal().x()) + projMatrix(2,0)) / projMatrix(0,0);
		qVec.y() = (osg::signOrZero(plane.getNormal().y()) + projMatrix(2,1)) / projMatrix(1,1);
		qVec.z() = -1;
		qVec.w() = (1 + projMatrix(2,2)) / projMatrix(3,2);

		// Calculate the scaled plane vector
		osg::Vec4f c = plane.asVec4() * (2.0 / (plane.asVec4() * qVec));

		// Replace the third row of the projection matrix
		projMatrix(0,2) = c.x();
		projMatrix(1,2) = c.y();
		projMatrix(2,2) = c.z() + 1;
		projMatrix(3,2) = c.w();

		mProjectionMatrix = projMatrix;
	}
}
