#pragma once

#include <osg/Camera>
#include <osg/Group>
#include <osg/Program>
#include <osg/Texture2D>

namespace skybolt {
namespace vis {

class ShadowMapGenerator
{
public:
	ShadowMapGenerator(osg::ref_ptr<osg::Program> shadowCasterProgram, int shadowMapId = 0);

	void update(const osg::Vec3& shadowCameraPosition, const osg::Vec3& lightDirection, const osg::Vec3& wrappedNoiseOrigin);
	float getRadiusWorldSpace() const { return mRadiusWorldSpace; }
	void setRadiusWorldSpace(double radius);

	float getDepthRangeWorldSpace() { return mDepthRangeWorldSpace; }
	void setDepthRangeWorldSpace(float range);

	void configureShadowReceiverStateSet(osg::StateSet& ss);

	osg::ref_ptr<osg::Texture2D> getTexture() const { return mTexture; }
	osg::ref_ptr<osg::Camera> getCamera() const { return mCamera; }

	osg::Matrix getShadowProjectionMatrix() const;

private:
	osg::ref_ptr<osg::Texture2D> mTexture;
	osg::ref_ptr<osg::Camera> mCamera;
	osg::ref_ptr<osg::Uniform> mCameraPositionUniform;
	osg::ref_ptr<osg::Uniform> mViewMatrixUniform;
	osg::ref_ptr<osg::Uniform> mViewProjectionMatrixUniform;
	osg::ref_ptr<osg::Uniform> mShadowProjectionMatrixUniform;
	float mRadiusWorldSpace = 2000;
	float mDepthRangeWorldSpace = 10000;
};

} // namespace vis
} // namespace skybolt
