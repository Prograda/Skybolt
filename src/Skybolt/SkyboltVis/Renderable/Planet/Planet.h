/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "PlanetSurface.h"
#include "SkyboltVis/RootNode.h"
#include "SkyboltVis/VisFactory.h"
#include "SkyboltVis/SkyboltVisFwd.h"
#include "SkyboltVis/Renderable/Atmosphere/Bruneton/BruentonAtmosphere.h"
#include "SkyboltVis/Renderable/Clouds/CloudRenderingParams.h"
#include "SkyboltVis/Renderable/Clouds/VolumeClouds.h"
#include "SkyboltVis/Renderable/Planet/Features/PlanetFeaturesSource.h"
#include "SkyboltVis/Renderable/Forest/GpuForest.h"

#include <SkyboltSim/SkyboltSimFwd.h>
#include <SkyboltSim/Spatial/LatLon.h>
#include <SkyboltCommon/File/FileLocator.h>

#include <osg/MatrixTransform>
#include <osg/Uniform>
#include <osg/Vec3f>

#include <optional>

namespace skybolt {
namespace vis {

// TODO: Split out aspects of the planet into different entity components (surface, atmosphere etc)
struct PlanetConfig
{
	px_sched::Scheduler* scheduler;
	const ShaderPrograms* programs;
	Scene* scene;
	float innerRadius;

	// Planet surface
	std::optional<PlanetTileSources> planetTileSources;
	DetailMappingTechniquePtr detailMappingTechnique;
	//! If true, height map edge texels are assumed to run along tile edges.
	//! If false, height map edge texels are assumed to be be offset half a texel inside the tile.
	bool heightMapTexelsOnTileEdge = false;

	// Atmosphere
	std::optional<BruentonAtmosphereConfig> atmosphereConfig;
	bool skyVisible = true;

	// Ocean
	VisFactoryRegistry* visFactoryRegistry; //!< Not null if waterEnabled = true
	bool waterEnabled = true;

	// Features (buildings etc)
	BuildingTypesPtr buildingTypes; //!< optional
	file::FileLocator fileLocator;
	std::vector<file::Path> featureTreeFiles;
	std::string featureTilesDirectoryRelAssetPackage;

	// Clouds
	osg::ref_ptr<osg::Texture2D> cloudsTexture; //!< Set to null to disable clouds
	vis::CloudRenderingParams cloudRenderingParams;

	// Forest
	std::optional<ForestParams> forestParams;
};

class BruentonAtmosphere;
class MyPlanetSurfaceListener;
class ReflectionCameraController;
class WaterMaterial;

class Planet : public RootNode
{
	friend MyPlanetSurfaceListener;
public:
	Planet(const PlanetConfig& config);
	~Planet();

	PlanetSurface* getSurface() const { return mPlanetSurface.get(); } //!< May returns null

	PlanetFeatures* getPlanetFeatures() const { return mPlanetFeatures.get(); } //!< May return null

	osg::ref_ptr<BruentonAtmosphere> getAtmosphere() const; //!< May return null

	osg::ref_ptr<WaterMaterial> getWaterMaterial() const; //!< May return null

	void setJulianDate(double date)
	{
		mJulianDate = date;
	}

	void setCloudsVisible(bool visible);
	bool getCloudsVisible(void) const { return mCloudsVisible; }

	VolumeCloudsPtr getClouds() const { return mVolumeClouds; }

	//! If set, clouds will have uniform coverage across planet.
	//! If not set, coverage will be governed by cloud texture.
	void setCloudCoverageFraction(std::optional<float> cloudCoverageFraction);
	std::optional<float> getCloudCoverageFraction() const { return mCloudCoverageFraction; }

	float calcAtmosphericDensity(const osg::Vec3f& position) const;

	double getInnerRadius() const { return mInnerRadius; }

public:
	// RootNode Implementation
	void setPosition(const osg::Vec3d& position) override;
	void setOrientation(const osg::Quat& orientation) override;
	void setTransform(const osg::Matrix& m) override  { mTransform->setMatrix(m); }

	osg::Vec3d getPosition() const override { return mTransform->getMatrix().getTrans(); }
	osg::Quat getOrientation() const override { return mTransform->getMatrix().getRotate(); }
	osg::Matrix getTransform() const override  { return mTransform->getMatrix(); }

	osg::Node* _getNode() const override;

	void updatePreRender(const CameraRenderContext& context) override;

private:
	Scene* mScene;
	osg::ref_ptr<WaterMaterial> mWaterMaterial; //!< May be null
	OceanPtr mOcean;
	std::unique_ptr <ReflectionCameraController> mReflectionCameraController;
	
	std::unique_ptr<PlanetSurface> mPlanetSurface; //!< May be null
	std::shared_ptr<PlanetSky> mPlanetSky;
	std::unique_ptr<PlanetFeatures> mPlanetFeatures; //!< May be null
	VolumeCloudsPtr mVolumeClouds;
	osg::ref_ptr<BruentonAtmosphere> mAtmosphere;

	osg::Uniform* mPlanetCenterUniform;
	osg::Uniform* mPlanetMatrixInvUniform;

	double mInnerRadius;
	std::optional<float> mAtmosphereScaleHeight;
	osg::ref_ptr<osg::Group> mPlanetGroup;
	osg::ref_ptr<osg::MatrixTransform> mTransform;
	osg::ref_ptr<osg::Group> mShadowSceneGroup;
	std::unique_ptr<MyPlanetSurfaceListener> mPlanetSurfaceListener;
	double mJulianDate = 0;
	osg::Uniform* mCloudDisplacementMetersUniform;
	osg::Uniform* mCloudCoverageFractionUniform;
	std::optional<float> mCloudCoverageFraction;

	bool mCloudsVisible = false;
};

} // namespace vis
} // namespace skybolt
