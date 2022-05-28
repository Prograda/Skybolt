/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#pragma once

#include "SkyboltEngine/SkyboltEngineFwd.h"
#include "SkyboltVisFwd.h"
#include "VisObject.h"

#include <osg/ClipNode>
#include <osg/Group>

namespace skybolt {
namespace vis {

class Scene : public VisObject
{
public:
	Scene();
	~Scene();

	// @param object can be added to multiple scenes
	void addObject(VisObject* object);
	void removeObject(VisObject* object);

	void addClipPlane(osg::ClipPlane* plane);
	void removeClipPlane(osg::ClipPlane* plane);

	//! @returns the direction the primary light is shining
	osg::Vec3f getPrimaryLightDirection() const;
	Light* getPrimaryLight() const { return mPrimaryLight; }

	void translateNoiseOrigin(const osg::Vec3f& offset);

	float calcAtmosphericDensity(const osg::Vec3f& position) const;

	osg::Vec3f getAmbientLightColor() const;
	void setAmbientLightColor(const osg::Vec3f& color);

	const osg::Vec3& getWrappedNoiseOrigin() const {return mWrappedNoiseOrigin;}

	osg::Group* _getGroup() const {return mGroup;} //!< Node under the root containing the scene objects

public: // VisObject interface
	void updatePreRender(const RenderContext& context);
	osg::Node* _getNode() const override {return mClipNode;} //!< The root node

private:
	std::vector<VisObject*> mObjects;
	osg::ref_ptr<osg::Group> mGroup;
	osg::ref_ptr<osg::ClipNode> mClipNode;
	Light* mPrimaryLight;
	Planet* mPrimaryPlanet;
	osg::Uniform* mCameraPositionUniform;
	osg::Uniform* mViewCameraPositionUniform; //!< Position of final view camera. Useful for getting view camera info in render targets other than the main view.
	osg::Uniform* mCameraCenterDirectionUniform;
	osg::Uniform* mCameraUpDirectionUniform;
	osg::Uniform* mCameraRightDirectionUniform;
	osg::Uniform* mViewMatrixUniform;
	osg::Uniform* mViewProjectionMatrixUniform;
	osg::Uniform* mProjectionMatrixUniform;
	osg::Uniform* mLightDirectionUniform;
	osg::Uniform* mDirectLightColorUniform;
	osg::Uniform* mAmbientLightColorUniform;
	osg::Uniform* mWrappedNoiseOriginUniform;
	osg::Uniform* mGroundIrradianceMultiplierUniform;
	osg::Vec3f mWrappedNoiseOrigin;
	float mWrappedNoisePeriod;
};

} // namespace vis
} // namespace skybolt
