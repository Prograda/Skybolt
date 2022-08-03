/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#include "DefaultRenderCameraViewport.h"
#include "RenderOperationSequence.h"
#include "RenderOperationUtil.h"

#include "SkyboltVis/Camera.h"
#include "SkyboltVis/GlobalSamplerUnit.h"
#include "SkyboltVis/Scene.h"
#include "SkyboltVis/Renderable/ScreenQuad.h"
#include "SkyboltVis/Renderable/Atmosphere/Bruneton/BruentonAtmosphereRenderOperation.h"
#include "SkyboltVis/Renderable/Clouds/CloudsRenderTexture.h"
#include "SkyboltVis/Renderable/Clouds/CloudsTemporalUpscaling.h"
#include "SkyboltVis/Renderable/Clouds/VolumeCloudsComposite.h"
#include "SkyboltVis/Renderable/Planet/Planet.h"
#include "SkyboltVis/Renderable/Water/WaterMaterialRenderOperation.h"
#include "SkyboltVis/RenderOperation/RenderEnvironmentMap.h"
#include "SkyboltVis/RenderOperation/RenderOperationOrder.h"
#include "SkyboltVis/Shadow/CascadedShadowMapGenerator.h"
#include "SkyboltVis/Shadow/ShadowHelpers.h"
#include "SkyboltVis/Shadow/ShadowMapGenerator.h"
#include "SkyboltVis/Shader/ShaderProgramRegistry.h"

namespace skybolt {
namespace vis {

static std::shared_ptr<ScreenQuad> createFullscreenQuad(const osg::ref_ptr<osg::Program>& program)
{
	osg::ref_ptr<osg::StateSet> stateSet = new osg::StateSet();
	stateSet->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
	stateSet->setMode(GL_CULL_FACE, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
	stateSet->setAttribute(program);
	return std::make_shared<ScreenQuad>(stateSet);
}

static osg::ref_ptr<RenderTexture> createMainPassTexture()
{
	RenderTextureConfig config;
	config.colorTextureFactories = { createScreenTextureFactory(GL_RGBA16F_ARB) };
	config.depthTextureFactory = createScreenTextureFactory(GL_DEPTH_COMPONENT);
	config.multisampleSampleCount = osg::DisplaySettings::instance()->getNumMultiSamples();

	return new RenderTexture(config);
}

DefaultRenderCameraViewport::DefaultRenderCameraViewport(const DefaultRenderCameraViewportConfig& config) :
	mScene(config.scene)
{
	setStateSet(mScene->getStateSet());

	mMainPassTexture = createMainPassTexture();
	mMainPassTexture->setScene(mScene->getBucketGroup(Scene::Bucket::Default));

	mFinalRenderTarget = createDefaultRenderTarget();
	mFinalRenderTarget->setScene(createFullscreenQuad(config.programs->getRequiredProgram("compositeFinal"))->_getNode());
	mFinalRenderTarget->setRelativeRect(config.relativeRect);

	mSequence = std::make_unique<RenderOperationSequence>();
	addChild(mSequence->getRootNode());

	mSequence->addOperation(createRenderOperationFunction([this] (const RenderContext& context) {
		auto camera = mMainPassTexture->getCamera();
		if (camera)
		{
			CameraRenderContext cameraContext(*camera);
			cameraContext.targetDimensions = context.targetDimensions;
			cameraContext.lightDirection = -mScene->getPrimaryLightDirection();
			cameraContext.atmosphericDensity = mScene->calcAtmosphericDensity(camera->getPosition());
			mScene->updatePreRender(cameraContext);

			if (!mMainPassTexture->getOutputTextures().empty())
			{
				mFinalRenderTarget->getOrCreateStateSet()->setTextureAttributeAndModes(0, mMainPassTexture->getOutputTextures().front());
			}
		}

	}), (int)RenderOperationOrder::PrepareScene);

	mSequence->addOperation(new BruentonAtmosphereRenderOperation(mScene), (int)RenderOperationOrder::PrecomputeAtmosphere);
	mSequence->addOperation(new RenderEnvironmentMap(mScene, *config.programs), (int)RenderOperationOrder::EnvironmentMap);
	mSequence->addOperation(new WaterMaterialRenderOperation(mScene, *config.programs), (int)RenderOperationOrder::WaterMaterial);

	// Shadows
	if (config.shadowParams)
	{
		CascadedShadowMapGeneratorConfig c;
		c.cascadeBoundingDistances = config.shadowParams->cascadeBoundingDistances;
		c.textureSize = config.shadowParams->textureSize;

		mShadowMapGenerator = std::make_unique<CascadedShadowMapGenerator>(c);

		{
			osg::StateSet* ss = mScene->getStateSet();
			ss->setDefine("ENABLE_SHADOWS");
		}

		osg::ref_ptr<osg::Group> shadowSceneGroup(new osg::Group);
		shadowSceneGroup->getOrCreateStateSet()->setDefine("CAST_SHADOWS");

		osg::ref_ptr<osg::Group> defaultGroup = mScene->getBucketGroup(Scene::Bucket::Default);
		shadowSceneGroup->addChild(defaultGroup);
		for (const auto& generator : mShadowMapGenerator->getGenerators())
		{
			generator->setScene(shadowSceneGroup);
			mSequence->addOperation(generator, (int)RenderOperationOrder::ShadowMap);
		}

		osg::StateSet* ss = defaultGroup->getOrCreateStateSet();
		mShadowMapGenerator->configureShadowReceiverStateSet(*ss);
		addShadowMapsToStateSet(mShadowMapGenerator->getTextures(), *ss, int(GlobalSamplerUnit::ShadowCascade0));
	}

	// Clouds
	{
		osg::ref_ptr<RenderTexture> clouds = createCloudsRenderTexture(mScene);
		mSequence->addOperation(clouds, (int)RenderOperationOrder::Clouds);

		osg::ref_ptr<RenderOperation> cloudsTarget = clouds;
		if (config.cloudRenderingParams.enableTemporalUpscaling)
		{
			CloudsTemporalUpscalingConfig upscalingConfig;
			upscalingConfig.colorTextureProvider = [clouds] { return clouds->getOutputTextures().empty() ? nullptr : clouds->getOutputTextures()[0]; };
			upscalingConfig.depthTextureProvider = [clouds] { return clouds->getOutputTextures().empty() ? nullptr : clouds->getOutputTextures()[1]; };
			upscalingConfig.scene = mScene;
			upscalingConfig.upscalingProgram = config.programs->getRequiredProgram("cloudsTemporalUpscaling");
			mCloudsUpscaling = new CloudsTemporalUpscaling(upscalingConfig);
			mSequence->addOperation(mCloudsUpscaling, (int)RenderOperationOrder::Clouds);

			cloudsTarget = mCloudsUpscaling;
		}

		VolumeCloudsCompositeConfig compositeConfig;
		compositeConfig.compositorProgram = config.programs->getRequiredProgram("compositeClouds");
		compositeConfig.colorTextureProvider = [cloudsTarget] { return cloudsTarget->getOutputTextures().empty() ? nullptr : cloudsTarget->getOutputTextures()[0]; };
		compositeConfig.depthTextureProvider = [cloudsTarget] { return cloudsTarget->getOutputTextures().empty() ? nullptr : cloudsTarget->getOutputTextures()[1]; };

		mCloudsComposite = createVolumeCloudsComposite(compositeConfig);
	}

	mSequence->addOperation(mMainPassTexture, (int)RenderOperationOrder::MainPass);
	mSequence->addOperation(mFinalRenderTarget, (int)RenderOperationOrder::FinalComposite);
}

DefaultRenderCameraViewport::~DefaultRenderCameraViewport()
{
	mScene->removeObject(mCloudsComposite);
}

void DefaultRenderCameraViewport::setCamera(const CameraPtr& camera)
{
	mCamera = camera;
	if (mCloudsUpscaling)
	{
		mCloudsUpscaling->setCamera(camera);
	}
	mMainPassTexture->setCamera(camera);
}

osg::ref_ptr<RenderTarget> DefaultRenderCameraViewport::getFinalRenderTarget() const
{
	return mFinalRenderTarget;
}

static bool getCloudsVisible(const Scene& scene)
{
	if (const auto& planet = scene.getPrimaryPlanet(); planet)
	{
		return planet->getCloudsVisible();
	}
	return false;
}

void DefaultRenderCameraViewport::updatePreRender(const RenderContext& renderContext)
{
	if (!mCamera)
	{
		return;
	}
	mSequence->updatePreRender(renderContext);

	if (mShadowMapGenerator)
	{
		mShadowMapGenerator->update(*mCamera, -mScene->getPrimaryLightDirection(), mScene->getWrappedNoiseOrigin());
	}

	bool cloudsVisible = getCloudsVisible(*mScene);
	if (cloudsVisible != mCloudsCompositeInScene)
	{
		mCloudsCompositeInScene = cloudsVisible;
		if (cloudsVisible)
		{
			mScene->addObject(mCloudsComposite);
		}
		else
		{
			mScene->removeObject(mCloudsComposite);
		}
	}
}

std::vector<osg::ref_ptr<osg::Texture>> DefaultRenderCameraViewport::getOutputTextures() const
{
	return mSequence->getOutputTextures();
}
	
} // namespace vis
} // namespace skybolt
