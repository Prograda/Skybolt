/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "ViewportStateSet.h"
#include "SkyboltVis/Camera.h"

namespace skybolt {
namespace vis {

ViewportStateSet::ViewportStateSet()
{
	mCameraPositionUniform = new osg::Uniform("cameraPosition", osg::Vec3f(0, 0, 0));
	addUniform(mCameraPositionUniform);

	mCameraCenterDirectionUniform = new osg::Uniform("cameraCenterDirection", osg::Vec3f(0, 0, 0));
	addUniform(mCameraCenterDirectionUniform);

	mCameraUpDirectionUniform = new osg::Uniform("cameraUpDirection", osg::Vec3f(0, 0, 0));
	addUniform(mCameraUpDirectionUniform);

	mCameraRightDirectionUniform = new osg::Uniform("cameraRightDirection", osg::Vec3f(0, 0, 0));
	addUniform(mCameraRightDirectionUniform);

	mFarClipDistanceUniform = new osg::Uniform("farClipDistance", 1e5f);
	addUniform(mFarClipDistanceUniform);

	mViewMatrixUniform = new osg::Uniform("viewMatrix", osg::Matrixf());
	addUniform(mViewMatrixUniform);

	mViewProjectionMatrixUniform = new osg::Uniform("viewProjectionMatrix", osg::Matrixf());
	addUniform(mViewProjectionMatrixUniform);

	mProjectionMatrixUniform = new osg::Uniform("projectionMatrix", osg::Matrixf());
	addUniform(mProjectionMatrixUniform);
}

void ViewportStateSet::update(const vis::Camera& camera)
{
	mCameraPositionUniform->set(osg::Vec3f(camera.getPosition()));
	mCameraCenterDirectionUniform->set(camera.getOrientation() * osg::Vec3f(1, 0, 0));
	mCameraUpDirectionUniform->set(camera.getOrientation() * osg::Vec3f(0, 0, -1));
	mCameraRightDirectionUniform->set(camera.getOrientation() * osg::Vec3f(0, 1, 0));
	mFarClipDistanceUniform->set(camera.getFarClipDistance());

	mViewMatrixUniform->set(camera.getViewMatrix());

	osg::Matrixf viewProj = camera.getViewMatrix() * camera.getProjectionMatrix();
	osg::Matrixf viewProjInv = osg::Matrix::inverse(viewProj);
	mViewProjectionMatrixUniform->set(viewProj);
	mProjectionMatrixUniform->set(camera.getProjectionMatrix());
}

} // namespace vis
} // namespace skybolt
