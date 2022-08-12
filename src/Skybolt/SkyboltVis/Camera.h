/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#pragma once

#include <osg/Camera>
#include <osg/Quat>
#include <osg/Vec3f>

#include <optional>

namespace skybolt {
namespace vis {

class Camera
{
public:
	Camera(float aspectRatio);
	~Camera();

	void setPosition(const osg::Vec3d &position);
	void setOrientation(const osg::Quat &orientaiton);

	osg::Vec3d getPosition() const {return mPosition;}
	osg::Quat getOrientation() const {return mOrientation;}

	void setNearClipDistance(float near);
	void setFarClipDistance(float far);
	void setFovY(float fovY);
	void setAspectRatio(float aspectRatio);

	float getNearClipDistance() const {return mNear;}
	float getFarClipDistance() const {return mFar;}
	float getFovY() const {return mFovY;}
	float getAspectRatio() const {return mAspectRatio;}

	void updateOsgCameraGeometry(osg::Camera& camera) const;

	osg::Matrix getViewMatrix() const { return mViewMatrix; }
	osg::Matrix getProjectionMatrix() const { return mProjectionMatrix; }

	void setVisibilityCategoryMask(uint32_t mask) { mCullMask = mask; }

private:
	void updateViewMatrix();
	void updateProjectionMatrix();

private:
	osg::Matrix mViewMatrix;
	osg::Matrix mProjectionMatrix;
	osg::Vec3f mPosition;
	osg::Quat mOrientation;
	float mAspectRatio;
	float mFovY;
	float mNear;
	float mFar;
	std::optional<osg::Plane> mObliqueClippingPlane;
	uint32_t mCullMask;
};

} // namespace vis
} // namespace skybolt
