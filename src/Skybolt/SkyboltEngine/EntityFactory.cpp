/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "EntityFactory.h"
#include "EngineRoot.h"
#include "EngineSettings.h"
#include "EngineStats.h"
#include "Components/PlanetElevationComponent.h"
#include "Components/TemplateNameComponent.h"
#include "Components/VisObjectsComponent.h"
#include "Scenario/ScenarioMetadataComponent.h"
#include "SimVisBinding/SimVisBinding.h"
#include "SimVisBinding/GeocentricToNedConverter.h"
#include "SimVisBinding/CameraSimVisBinding.h"
#include "SimVisBinding/CelestialObjectVisBinding.h"
#include "SimVisBinding/MoonVisBinding.h"
#include "SimVisBinding/ParticlesVisBinding.h"
#include "SimVisBinding/PlanetVisBinding.h"
#include "SimVisBinding/PolylineVisBinding.h"
#include "SimVisBinding/WakeBinding.h"

#include <SkyboltSim/JsonHelpers.h>
#include <SkyboltSim/World.h>
#include <SkyboltSim/WorldUtil.h>
#include <SkyboltSim/Components/CloudComponent.h>
#include <SkyboltSim/Components/MainRotorComponent.h>
#include <SkyboltSim/Components/NameComponent.h>
#include <SkyboltSim/Components/Node.h>
#include <SkyboltSim/Components/OceanComponent.h>
#include <SkyboltSim/Components/ParticleSystemComponent.h>
#include <SkyboltSim/Components/PlanetComponent.h>
#include <SkyboltSim/Components/PropellerComponent.h>
#include <SkyboltSim/Particles/ParticleSystem.h>
#include <SkyboltSim/Physics/Astronomy.h>
#include <SkyboltSim/Spatial/GreatCircle.h>

#include <SkyboltVis/Camera.h>
#include <SkyboltVis/Light.h>
#include <SkyboltVis/OsgImageHelpers.h>
#include <SkyboltVis/OsgStateSetHelpers.h>
#include <SkyboltVis/OsgTextureHelpers.h>
#include <SkyboltVis/RenderBinHelpers.h>
#include <SkyboltVis/TextureCache.h>
#include <SkyboltVis/Scene.h>
#include <SkyboltVis/Renderable/Particles.h>
#include <SkyboltVis/Renderable/Atmosphere/Bruneton/BruentonAtmosphere.h>
#include <SkyboltVis/Renderable/CameraRelativeBillboard.h>
#include <SkyboltVis/Renderable/Forest/GpuForest.h>
#include <SkyboltVis/Renderable/Polyline.h>
#include <SkyboltVis/Renderable/Planet/Planet.h>
#include <SkyboltVis/Renderable/Planet/Terrain.h>
#include <SkyboltVis/Renderable/Planet/Features/BuildingTypes.h>
#include <SkyboltVis/Renderable/Planet/Features/PlanetFeatures.h>
#include <SkyboltVis/Renderable/Planet/Tile/TileSource/JsonTileSourceFactory.h>
#include <SkyboltVis/Renderable/Stars/Starfield.h>
#include <SkyboltVis/Renderable/Model/Model.h>
#include <SkyboltVis/Renderable/Model/ModelFactory.h>
#include <SkyboltVis/Renderable/Water/WaterMaterial.h>
#include <SkyboltVis/Shader/ShaderProgramRegistry.h>

#include <SkyboltCommon/Random.h>
#include <SkyboltCommon/StringVector.h>
#include <SkyboltCommon/File/FileUtility.h>
#include <SkyboltCommon/Json/JsonHelpers.h>
#include <SkyboltCommon/Json/ReadJsonFile.h>
#include <SkyboltCommon/Math/MathUtility.h>
#include <filesystem>

#include <osg/BlendFunc>
#include <osg/Depth>
#include <osg/StateSet>
#include <osg/Texture2D>
#include <osgDB/ReadFile>
#include <osgDB/Registry>

using namespace skybolt;
using namespace skybolt::sim;

// Creates a 'textureizer map' which is a detail map that can be added
// to a lower detail base map.
// To produce a texturizer map, we take an original detail map, find the average,
// subtract the average, and add 0.5 to all channels. The result is a map that contains
// just the offsets to apply the original texture to a base map while preserving the average
// color of the base map.
// TODO: need to test this more to see if it is worth using. Currently unused.
static void convertSrgbToTexturizerMap(osg::Image& image)
{
	float alphaRejectionThreshold = -1.0; // don't reject alpha
	osg::Vec4f averageLinearSpace = vis::srgbToLinear(vis::averageSrgbColor(image, alphaRejectionThreshold));

	osg::Vec4f color;
	for (size_t t = 0; t < image.t(); ++t)
	{
		for (size_t s = 0; s < image.s(); ++s)
		{
			osg::Vec4f originalColor = image.getColor(s, t);
			osg::Vec4f c = vis::srgbToLinear(originalColor);
			c = c - averageLinearSpace + osg::Vec4f(0.5, 0.5, 0.5, 0.5);
			c = vis::linearToSrgb(c);
			c.a() = originalColor.a(); // perserve alpha
			image.setColor(c, s, t);
		}
	}
}

class MainRotorVisComponent : public SimVisBinding
{
public:
	MainRotorVisComponent(MainRotorComponent* rotor, const Positionable* attachedBody, const vis::RootNodePtr& visObject) :
		mRotor(rotor),
		mAttachedBody(attachedBody),
		mVisObject(visObject)
	{
		assert(mAttachedBody);
	}

	void syncVis(const GeocentricToNedConverter& converter) override
	{
		Vector3 pos = mAttachedBody->getPosition() + mAttachedBody->getOrientation() * mRotor->getPositionRelBody();

		mVisObject->setPosition(converter.convertPosition(pos));
		mVisObject->setOrientation(osg::Quat(mRotor->getRotationAngle(), osg::Vec3f(0, 0, 1)) * converter.convert(mAttachedBody->getOrientation() * mRotor->getTppOrientationRelBody()));
	}

private:
	MainRotorComponent* mRotor;
	const Positionable* mAttachedBody;
	vis::RootNodePtr mVisObject;
};

class PropellerVisComponent : public SimVisBinding
{
public:
	PropellerVisComponent(const PropellerComponent* propeller, const Positionable* attachedBody, const vis::RootNodePtr& visObject) :
		mPropeller(propeller),
		mAttachedBody(attachedBody),
		mVisObject(visObject)
	{
	}

	void syncVis(const GeocentricToNedConverter& converter) override
	{
		Vector3 pos = mAttachedBody->getPosition() + mAttachedBody->getOrientation() * mPropeller->getPositionRelBody();

		mVisObject->setPosition(converter.convertPosition(pos));
		mVisObject->setOrientation(osg::Quat(mPropeller->getRotationAngle(), osg::Vec3f(0, 0, 1)) * converter.convert(mAttachedBody->getOrientation() * mPropeller->getOrientationRelBody()));
	}

private:
	const PropellerComponent* mPropeller;
	const Positionable* mAttachedBody;
	vis::RootNodePtr mVisObject;
};

static osg::Vec3f readVec3f(const nlohmann::json& j)
{
	return osg::Vec3(j[0].get<double>(), j[1].get<double>(), j[2].get<double>());
}

static osg::Vec3f readOptionalVec3f(const nlohmann::json& j, const std::string& name, const osg::Vec3f& defaultValue)
{
	auto i = j.find(name);
	if (i != j.end())
	{
		return readVec3f(i.value());
	}
	return defaultValue;
}

static osg::Quat readQuat(const nlohmann::json& j)
{
	return osg::Quat(j.at("angleDeg").get<double>() * skybolt::math::degToRadD(), readVec3f(j.at("axis")));
}

static osg::Quat readOptionalQuat(const nlohmann::json& j, const std::string& name, const osg::Quat& defaultValue)
{
	auto i = j.find(name);
	if (i != j.end())
	{
		return readQuat(i.value());
	}
	return defaultValue;
}

static std::string getParentDirectory(const std::string& filename)
{
	std::filesystem::path p(filename);
	return p.parent_path().string();
}

static void registerAssetSearchDirectory(const std::string& filename)
{
	osgDB::FilePathList& list = osgDB::Registry::instance()->getDataFilePathList();
	auto i = std::find(list.begin(), list.end(), filename);
	if (i != list.end())
	{
		list.push_back(filename);
	}
}

static vis::ModelFactory::TextureRole toTextureRole(const std::string& str)
{
	static std::map<std::string, vis::ModelFactory::TextureRole> items = {
		{ "albedo", vis::ModelFactory::TextureRole::Albedo },
		{ "normal", vis::ModelFactory::TextureRole::Normal },
		{ "occlusionRoughnessMetalness", vis::ModelFactory::TextureRole::OcclusionRoughnessMetalness }
	};
	if (auto i = items.find(str); i != items.end())
	{
		return i->second;
	}
	throw std::runtime_error("Invalud texture role: " + str);
}

static std::vector<vis::ModelFactory::TextureRole> readTextureRoles(const nlohmann::json& json)
{
	std::vector<vis::ModelFactory::TextureRole> roles;

	auto textureRoleStrings = readOptionalVector(json, "textureRoles", std::vector<std::string>({"albedo", "normal"}));
	for (const std::string& roleString : textureRoleStrings)
	{
		roles.push_back(toTextureRole(roleString));
	}
	return roles;
}

static vis::ModelPtr createVisualModel(const nlohmann::json& json, vis::ModelFactory& factory)
{
	std::string filename = json.at("model").get<std::string>();
	std::vector<vis::ModelFactory::TextureRole> textureRoles = readTextureRoles(json);
	vis::ModelConfig config;
	config.node = factory.createModel(filename, textureRoles);

	registerAssetSearchDirectory(getParentDirectory(filename));

	return std::make_shared<vis::Model>(config);
}

static void loadVisualModel(Entity* entity, const EntityFactory::Context& context, const EntityFactory::VisContext& visContext, const VisObjectsComponentPtr& visObjectsComponent, const SimVisBindingsComponentPtr& simVisBindingComponent, const nlohmann::json& json)
{
	vis::ModelPtr model = createVisualModel(json, *visContext.modelFactory);
	visObjectsComponent->addObject(model);

	SimVisBindingPtr simVis(new SimpleSimVisBinding(entity, model,
		readOptionalVec3f(json, "positionRelBody", osg::Vec3f()),
		readOptionalQuat(json, "orientationRelBody", osg::Quat())
	));
	simVisBindingComponent->bindings.push_back(simVis);
}

static void loadVisualMainRotor(Entity* entity, const EntityFactory::Context& context, const EntityFactory::VisContext& visContext, const VisObjectsComponentPtr& visObjectsComponent, const SimVisBindingsComponentPtr& simVisBindingComponent, const nlohmann::json& json)
{
	vis::ModelPtr model = createVisualModel(json, *visContext.modelFactory);
	visObjectsComponent->addObject(model);

	auto rotor = entity->getFirstComponentRequired<MainRotorComponent>();
	auto node = entity->getFirstComponentRequired<Node>();

	SimVisBindingPtr simVis(new MainRotorVisComponent(rotor.get(), node.get(), model));
	simVisBindingComponent->bindings.push_back(simVis);
}

static void loadVisualTailRotor(Entity* entity, const EntityFactory::Context& context, const EntityFactory::VisContext& visContext, const VisObjectsComponentPtr& visObjectsComponent, const SimVisBindingsComponentPtr& simVisBindingComponent, const nlohmann::json& json)
{
	vis::ModelPtr model = createVisualModel(json, *visContext.modelFactory);
	visObjectsComponent->addObject(model);

	auto rotor = entity->getFirstComponentRequired<PropellerComponent>();
	auto node = entity->getFirstComponentRequired<Node>();

	SimVisBindingPtr simVis(new PropellerVisComponent(rotor.get(), node.get(), model));
	simVisBindingComponent->bindings.push_back(simVis);
}

static void loadVisualCamera(Entity* entity, const EntityFactory::Context& context, const EntityFactory::VisContext& visContext, const VisObjectsComponentPtr& visObjectsComponent, const SimVisBindingsComponentPtr& simVisBindingComponent, const nlohmann::json& json)
{
	vis::CameraPtr visCamera(new vis::Camera(1.0f));
	SimVisBindingPtr cameraSimVisBinding(new CameraSimVisBinding(entity, visCamera));
	simVisBindingComponent->bindings.push_back(cameraSimVisBinding);
}

static void loadParticleSystem(Entity* entity, const EntityFactory::Context& context, const EntityFactory::VisContext& visContext, const VisObjectsComponentPtr& visObjectsComponent, const SimVisBindingsComponentPtr& simVisBindingComponent, const nlohmann::json& json)
{
	NearestPlanetProvider nearestPlanetProvider = [world = context.simWorld] (const Vector3& position) {
		return findNearestEntityWithComponent<sim::PlanetComponent>(world->getEntities(), position).get();
	};

	ParticleEmitter::Params emitterParams;
	emitterParams.emissionRate = json.at("emissionRate");
	emitterParams.radius = json.at("radius");
	emitterParams.positionable = entity->getFirstComponentRequired<Node>();
	emitterParams.elevationAngle = DoubleRangeInclusive(json.at("elevationAngleMin"), json.at("elevationAngleMax"));
	emitterParams.speed = DoubleRangeInclusive(json.at("speedMin"), json.at("speedMax"));
	emitterParams.upDirection = readVector3(json.at("upDirection"));
	emitterParams.random = std::make_shared<Random>(/* seed */ 0);
	emitterParams.temperatureDegreesCelcius = readOptionalOrDefault(json, "initialTemperatureDegreesCelcius", 0.0);
	emitterParams.zeroAtmosphericDensityAlpha = readOptionalOrDefault(json, "zeroAtmosphericDensityAlpha", 1.0);
	emitterParams.earthSeaLevelAtmosphericDensityAlpha = readOptionalOrDefault(json, "earthSeaLevelAtmosphericDensityAlpha", 1.0);
	emitterParams.nearestPlanetProvider = nearestPlanetProvider;

	double lifetime = json.at("lifetime");

	ParticleIntegrator::Params integratorParams;
	integratorParams.lifetime = lifetime;
	integratorParams.radiusLinearGrowthPerSecond = json.at("radiusLinearGrowthPerSecond");
	integratorParams.atmosphericSlowdownFactor = json.at("atmosphericSlowdownFactor");
	integratorParams.heatTransferCoefficent = readOptional<float>(json, "heatTransferCoefficent");
	integratorParams.nearestPlanetProvider = nearestPlanetProvider;

	auto particleSystem = std::make_shared<ParticleSystem>(ParticleSystem::Operations({
		std::make_shared<ParticleIntegrator>(integratorParams), // integrate before emission ensure new particles emitted at end of time step
		std::make_shared<ParticleEmitter>(emitterParams),
		std::make_shared<ParticleKiller>(lifetime)
	}));
	entity->addComponent(std::make_shared<ParticleSystemComponent>(particleSystem));

	osg::ref_ptr<osg::Texture2D> texture = visContext.textureCache->getOrCreateTexture(json.at("albedoTexture"), [](const std::string& filename) {
		return vis::createTilingSrgbTexture(osgDB::readImageFile(filename));
	});

	auto program = visContext.programs->getRequiredProgram("particles");
	auto visParticles = std::make_shared<vis::Particles>(program, texture);
	visObjectsComponent->addObject(visParticles);

	SimVisBindingPtr binding(new ParticlesVisBinding(particleSystem, visParticles));
	simVisBindingComponent->bindings.push_back(binding);
}

struct PlanetStatsUpdater : vis::PlanetFeaturesListener, vis::QuadTreeTileLoaderListener, sim::Component
{
	PlanetStatsUpdater(EngineStats* stats, vis::Planet* planet)
		: mStats(stats), mPlanet(planet)
	{
		if (planet->getSurface())
		{
			planet->getSurface()->getTileLoaderListenable()->addListener(this);
		}

		if (planet->getPlanetFeatures())
		{
			planet->getPlanetFeatures()->addListener(this);
		}
	}

	~PlanetStatsUpdater()
	{
		if (mPlanet->getPlanetFeatures())
		{
			mPlanet->getPlanetFeatures()->removeListener(this);
		}

		if (mPlanet->getSurface())
		{
			mPlanet->getSurface()->getTileLoaderListenable()->removeListener(this);
		}

		mStats->terrainTileLoadQueueSize -= mOwnTilesLoading;
		mStats->featureTileLoadQueueSize -= mOwnFeaturesLoading;
	}

	void tileLoadRequested() override
	{
		++mStats->terrainTileLoadQueueSize;
		++mOwnTilesLoading;
	}

	void tileLoaded() override
	{
		--mStats->terrainTileLoadQueueSize;
		--mOwnTilesLoading;
	}

	void tileLoadCanceled() override
	{
		--mStats->terrainTileLoadQueueSize;
		--mOwnTilesLoading;
	}

	void featureLoadEnqueued() override
	{
		++mStats->featureTileLoadQueueSize;
		++mOwnFeaturesLoading;
	}

	void featureLoadDequeued() override
	{
		--mStats->featureTileLoadQueueSize;
		--mOwnFeaturesLoading;
	}

private:
	EngineStats* mStats;
	vis::Planet* mPlanet;
	size_t mOwnTilesLoading = 0;
	size_t mOwnFeaturesLoading = 0;
};

static osg::ref_ptr<osg::Texture2D> createCloudTexture(const std::string& filepath)
{
	osg::Image* image = vis::readImageWithCorrectOrientation(filepath);
	image->setInternalTextureFormat(vis::toSrgbInternalFormat(image->getInternalTextureFormat()));
	osg::Texture2D* texture = new osg::Texture2D(image);
	texture->setWrap(osg::Texture::WRAP_S, osg::Texture::REPEAT);
	return texture;
}

static osg::ref_ptr<osg::Texture2D> readTilingNonSrgbTexture(const std::string& filename)
{
	osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D(osgDB::readImageFile(filename));
	texture->setFilter(osg::Texture::FilterParameter::MIN_FILTER, osg::Texture::FilterMode::LINEAR_MIPMAP_LINEAR);
	texture->setFilter(osg::Texture::FilterParameter::MAG_FILTER, osg::Texture::FilterMode::LINEAR);
	texture->setWrap(osg::Texture::WRAP_S, osg::Texture::REPEAT);
	texture->setWrap(osg::Texture::WRAP_T, osg::Texture::REPEAT);
	return texture;
}

static void loadVisualPlanet(Entity* entity, const EntityFactory::Context& context, const EntityFactory::VisContext& visContext, const VisObjectsComponentPtr& visObjectsComponent, const SimVisBindingsComponentPtr& simVisBindingComponent, const nlohmann::json& json)
{
	auto planetComponent = entity->getFirstComponentRequired<PlanetComponent>();
	auto oceanComponent = entity->getFirstComponent<OceanComponent>();

	vis::PlanetConfig config;
	config.scheduler = context.scheduler;
	config.programs = visContext.programs;
	config.scene = visContext.scene;
	config.innerRadius = planetComponent->radius;
	config.visFactoryRegistry = visContext.visFactoryRegistry.get();
	config.waterEnabled = (oceanComponent != nullptr);
	config.fileLocator = context.fileLocator;
	
	{
		auto it = json.find("clouds");
		if (it != json.end())
		{
			const nlohmann::json& clouds = it.value();
			config.cloudsTexture = visContext.textureCache->getOrCreateTexture(clouds.at("map"), &createCloudTexture);
			config.cloudRenderingParams = getCloudRenderingParams(context.engineSettings);

			entity->addComponent(std::make_shared<CloudComponent>());
		}
	}

	{
		auto it = json.find("atmosphere");
		if (it != json.end())
		{
			const nlohmann::json& atmosphere = it.value();
			
			vis::BruentonAtmosphereConfig atmosphereConfig;
			atmosphereConfig.bottomRadius = readOptionalOrDefault(atmosphere, "bottomRadius", planetComponent->radius);
			atmosphereConfig.topRadius = readOptionalOrDefault(atmosphere, "topRadius", planetComponent->radius * 1.0094); // TODO: determine programatically from scale height

			if (auto coefficient = readOptional<double>(atmosphere, "earthReyleighScatteringCoefficient"))
			{
				atmosphereConfig.reyleighScatteringCoefficientCalculator = vis::createEarthReyleighScatteringCoefficientCalculator(*coefficient);
			}
			else if (auto table = readOptional<nlohmann::json>(atmosphere, "reyleighScatteringCoefficientTable"))
			{
				auto coefficients = table->at("coefficients");
				auto wavelengthsNm = table->at("wavelengthsNm");
				if (coefficients.size() != wavelengthsNm.size())
				{
					throw Exception("Must have equal number of coefficients and wavelengths");
				}

				atmosphereConfig.reyleighScatteringCoefficientCalculator = vis::createTableReyleighScatteringCoefficientCalculator(coefficients, wavelengthsNm);
			}
			else
			{
				throw Exception("Reyleigh scattering coefficient not defined");
			}

			atmosphereConfig.rayleighScaleHeight = atmosphere.at("rayleighScaleHeight").get<double>();
			atmosphereConfig.mieScaleHeight = atmosphere.at("mieScaleHeight").get<double>();
			atmosphereConfig.mieAngstromAlpha = atmosphere.at("mieAngstromAlpha").get<double>();
			atmosphereConfig.mieAngstromBeta = atmosphere.at("mieAngstromBeta").get<double>();
			atmosphereConfig.mieSingleScatteringAlbedo = atmosphere.at("mieSingleScatteringAlbedo").get<double>();
			atmosphereConfig.miePhaseFunctionG = atmosphere.at("miePhaseFunctionG").get<double>();
			atmosphereConfig.useEarthOzone = readOptionalOrDefault<bool>(atmosphere, "useEarthOzone", false);

			config.atmosphereConfig = atmosphereConfig;
		}
	}
	
	auto elevationComponent = entity->getFirstComponentRequired<PlanetElevationComponent>();
	config.heightMapTexelsOnTileEdge = elevationComponent->heightMapTexelsOnTileEdge;

	auto it = json.find("surface");
	if (it != json.end())
	{
		const nlohmann::json& layers = it.value();
		vis::PlanetTileSources planetTileSources;
		planetTileSources.elevation = elevationComponent->tileSource;

		auto it = layers.find("landMask");
		if (it != layers.end())
		{
			planetTileSources.landMask = context
				.tileSourceFactoryRegistry->getFactory(it->at("format"))(*it);
		}
		{
			nlohmann::json albedo = layers.at("albedo");
			planetTileSources.albedo = context
				.tileSourceFactoryRegistry->getFactory(albedo.at("format"))(albedo);
		}
		it = layers.find("attribute");
		if (it != layers.end())
		{
			planetTileSources.attribute = context
				.tileSourceFactoryRegistry->getFactory(it->at("format"))(*it);
		}
		it = layers.find("uniformDetail");
		if (it != layers.end())
		{
			std::string filename = it->at("texture");
			auto texture = vis::createTilingSrgbTexture(osgDB::readImageFile(filename));
			convertSrgbToTexturizerMap(*texture->getImage());
			auto technique = std::make_shared<vis::UniformDetailMappingTechnique>();
			technique->albedoDetailMap = texture;
			config.detailMappingTechnique = technique;
		}
		else
		{
			it = layers.find("albedoToDetail");
			if (it != layers.end())
			{
				auto technique = std::make_shared<vis::AlbedoDerivedDetailMappingTechnique>();
				technique->noiseMap = readTilingNonSrgbTexture("Environment/TerrainDetailNoise.png");

				auto textures = it->at("textures").items();
				for (const auto filename : textures)
				{
					auto texture = vis::createTilingSrgbTexture(osgDB::readImageFile(filename.value()));
					technique->albedoDetailMaps.push_back(texture);
				}

				config.detailMappingTechnique = technique;
			}
		}
		config.planetTileSources = planetTileSources;
	}

	{
		auto it = json.find("features");
		if (it != json.end())
		{
			std::string directory = it.value().at("directory");
			config.featureTreeFiles = getPathsInAssetPackages(context.assetPackagePaths, directory + "/tree.json");
			config.featureTilesDirectoryRelAssetPackage = directory;

			std::string buildingTypesFilename = it.value().at("buildingTypesFilename");
			buildingTypesFilename = skybolt::valueOrThrowException(context.fileLocator(buildingTypesFilename)).string();
			config.buildingTypes = vis::createBuildingTypesFromJson(readJsonFile(buildingTypesFilename));
		}
	}

	{
		auto it = json.find("forest");
		if (it != json.end())
		{
			vis::ForestParams params;
			params.forestGeoVisibilityRange = it.value().at("treeVisibilityRangeMeters");
			params.treesPerLinearMeter = it.value().at("treesPerLinearMeter");
			params.minTileLodLevelToDisplayForest = it.value().at("minLevel");
			params.maxTileLodLevelToDisplayForest = it.value().at("maxLevel");
			config.forestParams = params;
		}
	}

	vis::PlanetPtr visObject(new vis::Planet(config));
	visObjectsComponent->addObject(visObject);

	SimVisBindingPtr simVis(std::make_shared<PlanetVisBinding>(context.julianDateProvider, entity, visObject));
	simVisBindingComponent->bindings.push_back(simVis);

	if (visObject->getWaterMaterial())
	{
		auto binding = std::make_shared<WakeBinding>(context.simWorld, visObject->getWaterMaterial());
		simVisBindingComponent->bindings.push_back(binding);
	}

	std::shared_ptr<PlanetStatsUpdater> statsUpdater = std::make_shared<PlanetStatsUpdater>(context.stats, static_cast<vis::Planet*>(visObject.get()));
	entity->addComponent(statsUpdater);
}

using VisComponentLoader = std::function<void(Entity*, const EntityFactory::Context&, const EntityFactory::VisContext&, VisObjectsComponentPtr&, const SimVisBindingsComponentPtr&, const nlohmann::json&)>;

const ScenarioObjectPath& skybolt::getDefaultEntityScenarioObjectDirectory()
{
	static ScenarioObjectPath d = {"Platforms"};
	return d;
}

static std::shared_ptr<ScenarioMetadataComponent> createDefaultEntityScenarioMetadataComponent()
{
	auto component = std::make_shared<ScenarioMetadataComponent>();
	component->directory = getDefaultEntityScenarioObjectDirectory();
	return component;
}

EntityPtr EntityFactory::createEntityFromJson(const nlohmann::json& json, const std::string& templateName, const std::string& instanceName, const Vector3& position, const Quaternion& orientation, EntityId id) const
{
	EntityPtr entity = std::make_shared<sim::Entity>((id != nullEntityId()) ? id : generateNextEntityId());

	// Create required components
	entity->addComponent(std::make_shared<NameComponent>(instanceName));
	entity->addComponent(std::make_shared<TemplateNameComponent>(templateName));

	VisObjectsComponentPtr visObjectsComponent;
	SimVisBindingsComponentPtr simVisBindingComponent;
	if (mContext.visContext)
	{
		visObjectsComponent = std::make_shared<VisObjectsComponent>(mContext.visContext->scene);
		entity->addComponent(visObjectsComponent);

		simVisBindingComponent = std::make_shared<SimVisBindingsComponent>();
		entity->addComponent(simVisBindingComponent);
}

	// Create additional components from json
	ComponentFactoryContext componentFactoryContext;
	componentFactoryContext.julianDateProvider = mContext.julianDateProvider;
	componentFactoryContext.scheduler = mContext.scheduler;
	componentFactoryContext.simWorld = mContext.simWorld;
	componentFactoryContext.entityFactory = this;
	componentFactoryContext.stats = mContext.stats;
	componentFactoryContext.tileSourceFactoryRegistry = mContext.tileSourceFactoryRegistry;

	const nlohmann::json& components = json.at("components");
	for (const auto& component : components)
	{
		for (nlohmann::json::const_iterator componentIt = component.begin(); componentIt != component.end(); ++componentIt)
		{
			std::string key = componentIt.key();
			const nlohmann::json& content = componentIt.value();
			
			// Sim components
			{
				auto it = mContext.componentFactoryRegistry->find(key);
				if (it != mContext.componentFactoryRegistry->end())
				{
					ComponentFactoryPtr factory = it->second;
					auto newComponent = factory->create(entity.get(), componentFactoryContext, content);
					if (newComponent)
					{
						entity->addComponent(newComponent);
					}
				}
			}
			// Vis components
			if (mContext.visContext)
			{
				static std::map<std::string, VisComponentLoader> visComponentLoaders =
				{
					{ "camera", loadVisualCamera },
					{ "particleSystem", loadParticleSystem },
					{ "visualModel", loadVisualModel },
					{ "visualMainRotor", loadVisualMainRotor },
					{ "visualTailRotor", loadVisualTailRotor },
					{ "visualPlanet", loadVisualPlanet }
				};

				auto it = visComponentLoaders.find(key);
				if (it != visComponentLoaders.end())
				{
					assert(visObjectsComponent);
					assert(simVisBindingComponent);
					it->second(entity.get(), mContext, *mContext.visContext, visObjectsComponent, simVisBindingComponent, content);
				}
			}
		}
	}

	// Add default ScenarioMetadataComponent if one wasn't in the json file
	if (!entity->getFirstComponent<ScenarioMetadataComponent>())
	{
		entity->addComponent(createDefaultEntityScenarioMetadataComponent());
	}

	// Initialise entity pose
	if (Node* node = entity->getFirstComponent<Node>().get(); node)
	{
		node->setPosition(position);
		node->setOrientation(orientation);
	}

	return entity;
}

static ScenarioObjectPath readScenarioObjectDirectory(const nlohmann::json& json)
{
	ScenarioObjectPath result;
	ifChildExists(json, "components", [&](const nlohmann::json& components) {
		for (const auto& component : components)
		{
			ifChildExists(component, "scenarioMetadata", [&] (const nlohmann::json& c) {
				result = parseStringList(c.at("scenarioObjectDirectory").get<std::string>(), "/");
			});
		}
	});
	if (result.empty())
	{
		return getDefaultEntityScenarioObjectDirectory();
	}
	return result;
}

EntityFactory::EntityFactory(const EntityFactory::Context& context, const std::vector<std::filesystem::path>& entityFilenames) :
	mContext(context)
{
	assert(context.julianDateProvider);
	assert(context.programs);
	assert(context.simWorld);
	assert(context.stats);
	assert(context.tileSourceFactoryRegistry);
	assert(context.scene);

	if (context.visContext)
	{
		mBuiltinTemplates = {
			{"SunBillboard", [this] {return createSun(*mContext.visContext); }},
			{"MoonBillboard", [this] {return createMoon(*mContext.visContext); }},
			{"Stars", [this] {return createStars(*mContext.visContext); }}
		};
	}

	for (const std::filesystem::path& filename : entityFilenames)
	{
		std::string name = filename.stem().string();
		nlohmann::json json = readJsonFile(filename.string());
		mTemplateJsonMap[name] = json;
		mTemplateNames.push_back(name);
		mTemplateDirectories[name] = readScenarioObjectDirectory(json);
	}
}

EntityPtr EntityFactory::createEntity(const std::string& templateName, const std::string& nameIn, const Vector3& position, const Quaternion& orientation, EntityId id) const
{
	{
		auto i = mTemplateJsonMap.find(templateName);
		if (i != mTemplateJsonMap.end())
		{
			std::string instanceName = nameIn.empty() ? createUniqueObjectName(templateName) : nameIn;
			try
			{
				return createEntityFromJson(i->second, templateName, instanceName, position, orientation, id);
			}
			catch (const std::exception& e)
			{
				throw Exception("Error loading '" + templateName + "': " + e.what());
			}
		}
	}

	// Try builtin types
	{
		auto i = mBuiltinTemplates.find(templateName);
		if (i != mBuiltinTemplates.end())
		{
			return i->second();
		}
	}

	throw std::runtime_error("Invalid templateName: " + templateName);
}

const float sunDistance = 10000;
const float moonDistance = sunDistance;
const float sunDiameter = 2.0f * tan(skybolt::math::degToRadF() * 0.53f * 0.5f) * sunDistance;
const float moonDiameter = 2.0f * tan(skybolt::math::degToRadF() * 0.52f * 0.5f) * moonDistance;

static osg::ref_ptr<osg::StateSet> createCelestialBodyStateSet(const osg::ref_ptr<osg::Program>& program, const osg::ref_ptr<osg::Image>& image)
{
	osg::StateSet* ss = new osg::StateSet;
	ss->setAttribute(program);
	ss->setMode(GL_CULL_FACE, osg::StateAttribute::OFF);

	osg::Depth* depth = new osg::Depth;
	depth->setWriteMask(false);
	ss->setAttributeAndModes(depth, osg::StateAttribute::ON);

	osg::ref_ptr<osg::Texture2D> texture = image ? vis::createSrgbTexture(image) : nullptr;
	ss->setTextureAttributeAndModes(0, texture);
	ss->addUniform(vis::createUniformSampler2d("albedoSampler", 0));

	vis::setRenderBin(*ss, vis::RenderBinId::CelestialBody);

	return ss;
}

EntityPtr EntityFactory::createSun(const EntityFactory::VisContext& visContext) const
{
	osg::ref_ptr<osg::StateSet> ss = createCelestialBodyStateSet(
		visContext.programs->getRequiredProgram("sun"),
		osgDB::readImageFile("Environment/Space/SunDisc.png"));

	osg::ref_ptr<osg::BlendFunc> blendFunc = new osg::BlendFunc;
	ss->setAttributeAndModes(blendFunc);

	EntityPtr object(new Entity(generateNextEntityId()));
	object->addComponent(std::make_shared<Node>());

	float diameterScale = 1.15f; // account for disk in texture being slightly smaller than texture size
	vis::RootNodePtr node(new vis::CameraRelativeBillboard(ss, sunDiameter * diameterScale, sunDiameter * diameterScale, sunDistance));

	SimVisBindingsComponentPtr simVisBindingComponent(new SimVisBindingsComponent);
	object->addComponent(simVisBindingComponent);

	SimVisBindingPtr simVis(new CelestialObjectVisBinding(mContext.julianDateProvider, calcSunEclipticPosition, node));
	simVisBindingComponent->bindings.push_back(simVis);

	VisObjectsComponentPtr visObjectsComponent(new VisObjectsComponent(visContext.scene));
	visObjectsComponent->addObject(node);
	object->addComponent(visObjectsComponent);

	vis::LightPtr light(new vis::Light(osg::Vec3f(-1,0,0)));
	visObjectsComponent->addObject(light);

	{ // TODO: reused sun ecliptic position calculated for the billboard above to avoid recalculating
		SimVisBindingPtr simVis(new CelestialObjectVisBinding(mContext.julianDateProvider, calcSunEclipticPosition, light));
		simVisBindingComponent->bindings.push_back(simVis);
	}

	return object;
}

EntityPtr EntityFactory::createMoon(const EntityFactory::VisContext& visContext) const
{
	osg::ref_ptr<osg::StateSet> ss = createCelestialBodyStateSet(
		visContext.programs->getRequiredProgram("moon"),
		osgDB::readImageFile("Environment/Space/MoonDisc.jpg"));

	osg::Uniform* moonPhaseUniform = new osg::Uniform("moonPhase", 0.5f);
	ss->addUniform(moonPhaseUniform);

	EntityPtr object(new Entity(generateNextEntityId()));
	object->addComponent(std::make_shared<Node>());

	vis::RootNodePtr node(new vis::CameraRelativeBillboard(ss, moonDiameter, moonDiameter, moonDistance));

	SimVisBindingsComponentPtr simVisBindingComponent(new SimVisBindingsComponent);
	SimVisBindingPtr simVis(new MoonVisBinding(mContext.julianDateProvider, moonPhaseUniform, node));
	simVisBindingComponent->bindings.push_back(simVis);
	object->addComponent(simVisBindingComponent);

	VisObjectsComponentPtr visObjectsComponent(new VisObjectsComponent(visContext.scene));
	visObjectsComponent->addObject(node);
	object->addComponent(visObjectsComponent);

	return object;
}

EntityPtr EntityFactory::createStars(const EntityFactory::VisContext& visContext) const
{
	vis::StarfieldConfig config;
	config.program = visContext.programs->getRequiredProgram("starfield");
	vis::RootNodePtr starfield(new vis::Starfield(config));

	auto calcStarfieldEclipticPosition = [](double julianDate) { return LatLon(0, 0); };

	EntityPtr object(new Entity(generateNextEntityId()));
	object->addComponent(std::make_shared<Node>());

	SimVisBindingsComponentPtr simVisBindingComponent(new SimVisBindingsComponent);
	SimVisBindingPtr simVis(new CelestialObjectVisBinding(mContext.julianDateProvider, calcStarfieldEclipticPosition, starfield));
	simVisBindingComponent->bindings.push_back(simVis);
	object->addComponent(simVisBindingComponent);

	VisObjectsComponentPtr visObjectsComponent(new VisObjectsComponent(visContext.scene));
	visObjectsComponent->addObject(starfield);
	object->addComponent(visObjectsComponent);

	return object;
}

const skybolt::ScenarioObjectPath& EntityFactory::getScenarioObjectDirectoryForTemplate(const std::string& templateName) const
{
	if (auto i = mTemplateDirectories.find(templateName); i != mTemplateDirectories.end())
	{
		return i->second;
	}
	return getDefaultEntityScenarioObjectDirectory();
}

std::string EntityFactory::createUniqueObjectName(const std::string& baseName) const
{
	for (int i = 1; i < INT_MAX; ++i)
	{
		std::string name = baseName + std::to_string(i);
		if (mContext.simWorld->findObjectByName(name) == nullptr)
		{
			return name;
		}
	}
	throw skybolt::Exception("Could not create unique object name from base name: " + baseName);
}

sim::EntityId EntityFactory::generateNextEntityId() const
{
	++mNextEntityId.entityId;
	return mNextEntityId;
}