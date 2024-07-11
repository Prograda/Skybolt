/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltVis/RenderOperation/RenderTexture.h"
#include <osg/PrimitiveSet>
#include <osg/Texture2D>

namespace skybolt {
namespace vis {

using TextureProvider = std::function<osg::ref_ptr<osg::Texture>()>;

struct CloudsTemporalUpscalingConfig
{
	ScenePtr scene;
	osg::ref_ptr<osg::Program> upscalingProgram;
	TextureProvider colorTextureProvider;
	TextureProvider depthTextureProvider;
};

class CloudsTemporalUpscaling : public RenderOperation
{
public:
	CloudsTemporalUpscaling(const CloudsTemporalUpscalingConfig& config);

	void updatePreRender(const RenderContext& renderContext) override;

	void setCamera(const vis::CameraPtr& camera) { mCamera = camera; }

	std::vector<osg::ref_ptr<osg::Texture>> getOutputTextures() const override;

private:
	ScenePtr mScene;
	TextureProvider mColorTextureProvider;
	TextureProvider mDepthTextureProvider;
	osg::ref_ptr<RenderTexture> mUpscaledColorTexture[2];
	osg::ref_ptr<osg::Uniform> mReprojectionMatrixUniform;
	osg::ref_ptr<osg::Uniform> mInvReprojectionMatrixUniform;
	osg::ref_ptr<osg::Uniform> mFrameNumberUniform;
	osg::ref_ptr<osg::Uniform> mJitterOffsetUniform;
	osg::ref_ptr<osg::StateSet> mStateSet;

	vis::CameraPtr mCamera;
	osg::Matrix mPrevFrameModelViewProjXyMatrix; //!< model_matrix * view_matrix * xy_components_of_projection_matrix
	int mUpscaledOutputIndex = 1;
};

} // namespace vis
} // namespace skybolt
