/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#pragma once

#include "RenderCameraViewport.h"
#include "SkyboltVis/Rect.h"
#include "SkyboltVis/RenderContext.h"
#include "SkyboltVis/Renderable/Clouds/CloudsTemporalUpscaling.h"
#include "SkyboltVis/Renderable/Clouds/CloudRenderingParams.h"
#include "SkyboltVis/Shadow/ShadowParams.h"

#include <osg/Camera>
#include <osgViewer/Viewer>

#include <optional>

namespace skybolt {
namespace vis {

class VolumeCloudsComposite;
class ViewportStateSet;

struct DefaultRenderCameraViewportConfig
{
	ScenePtr scene;
	const ShaderPrograms* programs;
	std::optional<ShadowParams> shadowParams;
	CloudRenderingParams cloudRenderingParams;
	RectF relativeRect = RectF(0,0,1,1);
};

class DefaultRenderCameraViewport : public RenderCameraViewport
{
public:
	//! @param relativeRect is the viewport dimensions relative to parent window
	DefaultRenderCameraViewport(const DefaultRenderCameraViewportConfig& config);
	~DefaultRenderCameraViewport() override;
	
	void setCamera(const CameraPtr& camera) override;
	
	osg::ref_ptr<RenderTarget> getFinalRenderTarget() const override;

	void updatePreRender(const RenderContext& renderContext) override;

	std::vector<osg::ref_ptr<osg::Texture>> getOutputTextures() const override;

private:
	osg::ref_ptr<ViewportStateSet> mViewportStateSet;
	ScenePtr mScene;
	std::unique_ptr<RenderOperationSequence> mSequence;
	osg::ref_ptr<RenderTexture> mMainPassTexture;
	osg::ref_ptr<RenderTarget> mFinalRenderTarget;
	osg::ref_ptr<RenderTarget> mHudTarget;
	std::unique_ptr<class CascadedShadowMapGenerator> mShadowMapGenerator;

	osg::ref_ptr<CloudsTemporalUpscaling> mCloudsUpscaling; //!< May be null

	CameraPtr mCamera;
};

} // namespace vis
} // namespace skybolt
