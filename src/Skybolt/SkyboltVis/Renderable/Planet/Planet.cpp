/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "Planet.h"
#include "SkyboltVis/GlobalSamplerUnit.h"
#include "SkyboltVis/OsgImageHelpers.h"
#include "SkyboltVis/OsgTextureHelpers.h"
#include "SkyboltVis/RenderContext.h"
#include "SkyboltVis/Camera.h"
#include "SkyboltVis/Light.h"
#include "SkyboltVis/MatrixHelpers.h"
#include "SkyboltVis/OsgStateSetHelpers.h"
#include "SkyboltVis/Scene.h"
#include "SkyboltVis/VisibilityCategory.h"
#include "SkyboltVis/Shader/ShaderProgramRegistry.h"
#include "SkyboltVis/Renderable/Atmosphere/Bruneton/BruentonAtmosphere.h"
#include "SkyboltVis/Renderable/Clouds/CloudsRenderTexture.h"
#include "SkyboltVis/Renderable/Planet/PlanetSurface.h"
#include "SkyboltVis/Renderable/Planet/PlanetSky.h"
#include "SkyboltVis/Renderable/Planet/Terrain.h"
#include "SkyboltVis/Renderable/Planet/Tile/PlanetTileImagesLoader.h"
#include "SkyboltVis/Renderable/Planet/Features/PlanetFeatures.h"
#include "SkyboltVis/Renderable/Planet/Tile/OsgTileFactory.h"
#include "SkyboltVis/Renderable/Planet/Tile/TileTextureCache.h"
#include "SkyboltVis/Renderable/Billboard.h"
#include "SkyboltVis/Renderable/Water/ReflectionCameraController.h"
#include "SkyboltVis/Renderable/Water/Ocean.h"
#include "SkyboltVis/Renderable/Water/WaterMaterial.h"
#include "SkyboltVis/Renderable/Water/WaveHeightTextureGenerator.h"

#include <SkyboltSim/Physics/Astronomy.h>

#include <SkyboltCommon/Exception.h>
#include <SkyboltCommon/Math/MathUtility.h>

#include <osgDB/ReadFile>

#include <future>
#include <boost/log/trivial.hpp>

using namespace skybolt::sim;

namespace skybolt {
namespace vis {

static osg::ref_ptr<osg::Texture2D> createReflectionTexture(int textureWidth, int textureHeight)
{
	//Create the texture image
	osg::Image* image = new osg::Image();
	image->allocateImage(textureWidth,
		textureHeight,
		1,   // 2D texture is 1 pixel deep
		GL_RGBA,
		GL_UNSIGNED_BYTE);
	image->setInternalTextureFormat(GL_RGBA8);

	// Create the texture object
	osg::ref_ptr<osg::Texture2D> texture = createRenderTexture(textureWidth, textureHeight);
	texture->setResizeNonPowerOfTwoHint(false);
	texture->setImage(image);
	texture->setInternalFormat(GL_RGBA);
	texture->setFilter(osg::Texture2D::FilterParameter::MIN_FILTER, osg::Texture2D::FilterMode::LINEAR);
	texture->setFilter(osg::Texture2D::FilterParameter::MAG_FILTER, osg::Texture2D::FilterMode::LINEAR);
	texture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
	texture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);

	return texture;
}

class MyPlanetSurfaceListener : public PlanetSurfaceListener
{
public:
	MyPlanetSurfaceListener(Planet* planet) :
		mPlanet(planet)
	{}

	void tileAddedToSceneGraph(const skybolt::QuadTreeTileKey& key) override
	{
		mPlanet->mPlanetFeatures->onSurfaceTileAdded(key);
	}

	Planet* mPlanet;
};

static osg::ref_ptr<osg::Texture2D> createNonSrgbTextureWithoutMipmaps(const osg::ref_ptr<osg::Image>& image)
{
	osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D(image);
	texture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
	texture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
	texture->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
	texture->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
	texture->setNumMipmapLevels(0);
	texture->setUseHardwareMipMapGeneration(false);
	texture->setResizeNonPowerOfTwoHint(false); // tile can be non power of two, e.g for storing extra row/column of adjecent tile data
	return texture;
}

static OsgTileFactory::TileTextures createSurfaceTileTextures(TileTextureCache& cache, const PlanetTileImages& images)
{
	// We create some of these textures with mip-mapping disabled when it's not needed.
	// Generating mipmaps is expensive and we need tile textures to load as quickly as possible.
	OsgTileFactory::TileTextures textures;
	textures.height.texture = cache.getOrCreateTexture(TileTextureCache::TextureType::Height, images.heightMapImage.image, createNonSrgbTextureWithoutMipmaps);
	textures.height.key = images.heightMapImage.key;
	textures.normal = cache.getOrCreateTexture(TileTextureCache::TextureType::Normal, images.normalMapImage, createNonSrgbTextureWithoutMipmaps);
	textures.landMask = cache.getOrCreateTexture(TileTextureCache::TextureType::LandMask, images.landMaskImage, createNonSrgbTextureWithoutMipmaps);
	textures.albedo.texture = cache.getOrCreateTexture(TileTextureCache::TextureType::Albedo, images.albedoMapImage.image, createSrgbTexture);
	textures.albedo.key = images.albedoMapImage.key;

	if (images.attributeMapImage)
	{
		TileTexture attribute;
		attribute.texture = cache.getOrCreateTexture(TileTextureCache::TextureType::Attribute, images.attributeMapImage->image, createNonSrgbTextureWithoutMipmaps);
		attribute.key = images.attributeMapImage->key;
		textures.attribute = attribute;
	}
	return textures;
}

static GpuForestTileTextures createGpuForestTileTextures(TileTextureCache& cache, const PlanetTileImages& images)
{
	GpuForestTileTextures textures;
	textures.height.texture = cache.getOrCreateTexture(TileTextureCache::TextureType::Height, images.heightMapImage.image, createNonSrgbTextureWithoutMipmaps);
	textures.height.key = images.heightMapImage.key;
	if (images.attributeMapImage)
	{
		textures.attribute.texture = cache.getOrCreateTexture(TileTextureCache::TextureType::Attribute, images.attributeMapImage->image, createNonSrgbTextureWithoutMipmaps);
		textures.attribute.key = images.attributeMapImage->key;
	}
	return textures;
}

using TileTextureCachePtr = std::shared_ptr<TileTextureCache>;
static std::function<OsgTileFactory::TileTextures(const PlanetTileImages&)> createSurfaceTileTexturesProvider(const TileTextureCachePtr& cache)
{
	return [cache] (const PlanetTileImages& images) {
		return createSurfaceTileTextures(*cache, images);
	};
}

static std::function<GpuForestTileTextures(const TileImages&)> createGpuForestTileTexturesProvider(const TileTextureCachePtr& cache)
{
	return [cache] (const TileImages& images) {
		return createGpuForestTileTextures(*cache, static_cast<const PlanetTileImages&>(images));
	};
}

Planet::Planet(const PlanetConfig& config) :
	mScene(config.scene),
	mInnerRadius(config.innerRadius),
	mPlanetGroup(new osg::Group),
	mTransform(new osg::MatrixTransform),
	mPlanetSurfaceListener(new MyPlanetSurfaceListener(this))
{
	mPlanetGroup->addChild(mTransform);

	if (config.atmosphereConfig)
	{
		mAtmosphereScaleHeight = config.atmosphereConfig->rayleighScaleHeight;

		mAtmosphere = osg::ref_ptr<BruentonAtmosphere>(new BruentonAtmosphere(*config.atmosphereConfig));

		mTransform->getOrCreateStateSet()->setDefine("ENABLE_ATMOSPHERE");

		if (config.skyVisible)
		{
			PlanetSkyConfig skyConfig;
			skyConfig.program = config.programs->getRequiredProgram("sky");
			skyConfig.radius = config.atmosphereConfig->topRadius;
			mPlanetSky.reset(new PlanetSky(skyConfig));
			mScene->addObject(mPlanetSky);
		}
	}

	// Global uniforms, to be shared for everything in scene
	{
		osg::StateSet* ss = mScene->getStateSet();
		mPlanetCenterUniform = new osg::Uniform("planetCenter", osg::Vec3f(0, 0, 0));
		ss->addUniform(mPlanetCenterUniform);

		mPlanetMatrixInvUniform = new osg::Uniform("planetMatrixInv", osg::Matrixf());
		ss->addUniform(mPlanetMatrixInvUniform);

		ss->addUniform(new osg::Uniform("innerRadius", (float)mInnerRadius));

		mCloudDisplacementMetersUniform = new osg::Uniform("cloudDisplacementMeters", osg::Vec2f(0, 0));
		ss->addUniform(mCloudDisplacementMetersUniform);

		mCloudCoverageFractionUniform = new osg::Uniform("cloudCoverageFraction", 0.5f);
		ss->addUniform(mCloudCoverageFractionUniform);

		ss->addUniform(new osg::Uniform("solar_irradiance", BruentonAtmosphere::getSolarIrradiance()));
	}


	// Create terrain
	if (config.planetTileSources)
	{
		auto textureCache = std::make_shared<TileTextureCache>();
		std::shared_ptr<OsgTileFactory> osgTileFactory;
		{
			OsgTileFactoryConfig factoryConfig;
			factoryConfig.programs = config.programs;
			factoryConfig.planetRadius = mInnerRadius;
			factoryConfig.hasCloudShadows = config.cloudsTexture != nullptr;
			factoryConfig.detailMappingTechnique = config.detailMappingTechnique;
			factoryConfig.heightMapTexelsOnTileEdge = config.heightMapTexelsOnTileEdge;
			osgTileFactory = std::make_shared<OsgTileFactory>(factoryConfig);
		}

		GpuForestPtr forest;
		if (config.forestParams)
		{
			osg::ref_ptr<osg::Group> forestGroup = new osg::Group();

			vis::GpuForestConfig forestConfig;
			forestConfig.forestParams = *config.forestParams;
			forestConfig.parentGroup = forestGroup;
			forestConfig.parentTransform = mTransform;
			forestConfig.planetRadius = mInnerRadius;
			forestConfig.tileTexturesProvider = createGpuForestTileTexturesProvider(textureCache);
			forestConfig.programs = config.programs;
			forest = std::make_shared<vis::GpuForest>(forestConfig);

			mPlanetGroup->addChild(forestGroup);
		}

		PlanetSurfaceConfig surfaceConfig;
		surfaceConfig.scheduler = config.scheduler;
		surfaceConfig.programs = config.programs;
		surfaceConfig.radius = mInnerRadius;
		surfaceConfig.osgTileFactory = osgTileFactory;
		surfaceConfig.parentTransform = mTransform;
		surfaceConfig.gpuForest = forest;
		surfaceConfig.planetTileSources = *config.planetTileSources;
		surfaceConfig.oceanEnabled = config.waterEnabled;
		surfaceConfig.cloudsTexture = config.cloudsTexture;
		surfaceConfig.tileTexturesProvider = createSurfaceTileTexturesProvider(textureCache);

		mPlanetSurface.reset(new PlanetSurface(surfaceConfig));
	}

	// Create water state set
	if (config.waterEnabled)
	{
		auto it = config.visFactoryRegistry->find(VisFactoryType::WaveHeightTextureGenerator);
		if (it != config.visFactoryRegistry->end())
		{
			mWaterMaterial = new WaterMaterial([&]{
				WaterMaterialConfig c;
				c.factory = static_cast<const WaveHeightTextureGeneratorFactory*>(it->second.get());
				c.programs = config.programs;
				return c;
			}());

			// Create ocean
			OceanConfig oceanConfig;
			oceanConfig.oceanProgram = config.programs->getRequiredProgram("ocean");
			oceanConfig.waterStateSet = mWaterMaterial->getStateSet();

			mOcean.reset(new Ocean(oceanConfig));
			mOcean->setPosition(osg::Vec3f(0, 0, 0));
			mScene->addObject(mOcean);
		}
	}

	osg::ref_ptr<osg::Group> nonBuildingFeaturesGroup = new osg::Group();
	nonBuildingFeaturesGroup->setNodeMask(vis::VisibilityCategory::defaultCategories);
	mTransform->addChild(nonBuildingFeaturesGroup);

	osg::ref_ptr<osg::Group> buildingsGroup = new osg::Group();
	mTransform->addChild(buildingsGroup);

	// Features
	if (mWaterMaterial)
	{
		if (!config.featureTreeFiles.empty())
		{
			if (config.buildingTypes)
			{
				PlanetFeaturesParams params;
				params.scheduler = config.scheduler;
				params.treeFiles = config.featureTreeFiles;
				params.fileLocator = config.fileLocator;
				params.tilesDirectoryRelAssetPackage = config.featureTilesDirectoryRelAssetPackage;
				params.programs = config.programs;
				params.waterStateSet = mWaterMaterial->getStateSet();
				params.buildingTypes = config.buildingTypes;
				params.planetRadius = mInnerRadius;

				params.groups[PlanetFeaturesParams::groupsBuildingsIndex] = buildingsGroup;
				params.groups[PlanetFeaturesParams::groupsNonBuildingsIndex] = nonBuildingFeaturesGroup;
				mPlanetFeatures.reset(new PlanetFeatures(params));

				if (mPlanetSurface)
				{
					mPlanetSurface->Listenable<PlanetSurfaceListener>::addListener(mPlanetSurfaceListener.get());
				}
			}
			else
			{
				BOOST_LOG_TRIVIAL(error) << "Building types not defined";
			}
		}
	}

	// Clouds
	if (config.cloudsTexture)
	{
		{
			VolumeCloudsConfig cloudsConfig;
			cloudsConfig.program = config.programs->getRequiredProgram("volumeClouds");
			cloudsConfig.innerCloudLayerRadius = config.innerRadius + 3000;
			cloudsConfig.outerCloudLayerRadius = config.innerRadius + 8000;
			cloudsConfig.cloudsTexture = config.cloudsTexture;
			cloudsConfig.applyTemporalUpscalingJitter = config.cloudRenderingParams.enableTemporalUpscaling;
			mVolumeClouds = std::make_unique<VolumeClouds>(cloudsConfig);
		}

		setCloudsVisible(true);
		setCloudCoverageFraction(std::nullopt);

		osg::StateSet* ss = mScene->getStateSet();
		ss->setTextureAttributeAndModes((int)GlobalSamplerUnit::GlobalCloudAlpha, config.cloudsTexture);
		ss->addUniform(createUniformSampler2d("cloudSampler", (int)GlobalSamplerUnit::GlobalCloudAlpha));

		{
			osg::ref_ptr<osg::Texture> texture = new osg::Texture2D(osgDB::readImageFile("Environment/Noise.png"));
			texture->setWrap(osg::Texture::WRAP_S, osg::Texture::REPEAT);
			texture->setWrap(osg::Texture::WRAP_T, osg::Texture::REPEAT);
			ss->setTextureAttributeAndModes((int)GlobalSamplerUnit::CloudDetail2d, texture);
			ss->addUniform(createUniformSampler2d("coverageDetailSampler2", (int)GlobalSamplerUnit::CloudDetail2d));
		}
	}
}

Planet::~Planet()
{
	if (mOcean)
	{
		mScene->removeObject(mOcean);
	}

	if (mPlanetSky)
	{
		mScene->removeObject(mPlanetSky);
	}

	setCloudsVisible(false);

	// Remove listener to decouple shutdown order of mPlanetSurface and mPlanetSurfaceListener
	if (mPlanetSurface)
	{
		mPlanetSurface->Listenable<PlanetSurfaceListener>::removeListener(mPlanetSurfaceListener.get());
	}
}

osg::ref_ptr<class BruentonAtmosphere> Planet::getAtmosphere() const
{
	return mAtmosphere;
}

osg::ref_ptr<WaterMaterial> Planet::getWaterMaterial() const
{
	return mWaterMaterial;
}

void Planet::setCloudsVisible(bool visible)
{
	if (mVolumeClouds)
	{
		osg::StateSet* ss = mScene->getStateSet();
		if (visible && !mCloudsVisible)
		{
			mScene->addObject(mVolumeClouds, Scene::Bucket::Clouds);
			ss->setDefine("ENABLE_CLOUDS");
		}
		else if (!visible && mCloudsVisible)
		{
			mScene->removeObject(mVolumeClouds);
			ss->removeDefine("ENABLE_CLOUDS");
		}
	}
	mCloudsVisible = visible;
}

void Planet::setCloudCoverageFraction(std::optional<float> cloudCoverageFraction)
{
	mCloudCoverageFraction = cloudCoverageFraction;
	osg::StateSet* ss = mScene->getStateSet();
	if (cloudCoverageFraction)
	{
		mCloudCoverageFractionUniform->set(*cloudCoverageFraction);
		ss->removeDefine("USE_CLOUD_COVERAGE_MAP");
	}
	else
	{
		ss->setDefine("USE_CLOUD_COVERAGE_MAP");
	}
}

void Planet::setPosition(const osg::Vec3d &position)
{
	osg::Matrix mat = mTransform->getMatrix();
	mat.setTrans(position);
	mTransform->setMatrix(mat);
}

void Planet::setOrientation(const osg::Quat &orientation)
{
	osg::Matrix mat = mTransform->getMatrix();
	mat.setRotate(orientation);
	mTransform->setMatrix(mat);
}

osg::Node* Planet::_getNode() const
{
	return mPlanetGroup.get();
}

static osg::Quat getOrientationFromAzEl(const AzEl& azEl)
{
	return osg::Quat(azEl.elevation, osg::Vec3f(0, 1, 0)) * osg::Quat(azEl.azimuth, osg::Vec3f(0, 0, 1));
}

void Planet::updatePreRender(const CameraRenderContext& context)
{
	osg::Vec3d position = getPosition();
	osg::Vec3f positionFloat = osg::Vec3f(position);
	if (mPlanetSky)
	{
		mPlanetSky->setPosition(positionFloat);
	}
	
	if (mOcean)
	{
		mOcean->setPosition(osg::Vec3f(0, 0, position.z() - mInnerRadius));
	}
	
	if (mVolumeClouds)
	{
		mVolumeClouds->setPosition(positionFloat);
		mVolumeClouds->setOrientation(getOrientation());
	}

	mPlanetCenterUniform->set(positionFloat);

	osg::Matrixf planetMatrixInv;
	getOrientation().get(planetMatrixInv);
	planetMatrixInv = osg::Matrixf::inverse(planetMatrixInv);
	mPlanetMatrixInvUniform->set(planetMatrixInv);

	if (mPlanetSurface)
	{
		mPlanetSurface->updatePreRender(context);
	}

	if (mPlanetFeatures)
	{
		mPlanetFeatures->updatePreRender(context);
	}

	double julianDateSeconds = mJulianDate * 24.0 * 60.0 * 60.0;
	// Move clouds
	{
		float windSpeed = 10.0;
		osg::Vec2 displacement(0.0, -20000 + std::fmod(julianDateSeconds, 40000) * windSpeed);
		mCloudDisplacementMetersUniform->set(displacement);
	}

	if (mReflectionCameraController)
	{
		mReflectionCameraController->update(context.camera);
	}

	if (mWaterMaterial)
	{
		double altitude = (getPosition() - context.camera.getPosition()).length() - mInnerRadius;
		if (altitude < 40000) // TODO: ensure this value matches oceanMeshFadeoutEndDistance in Ocean.h shader file
		{
			mWaterMaterial->update(julianDateSeconds);
		}
	}
}

float Planet::calcAtmosphericDensity(const osg::Vec3f& position) const
{
	if (mAtmosphereScaleHeight)
	{
		float altitude = (position - getPosition()).length() - mInnerRadius;
		return 1.225f * std::min(1.0f, std::exp(-altitude / *mAtmosphereScaleHeight));
	}
	return 0.0f;
}

} // namespace vis
} // namespace skybolt
