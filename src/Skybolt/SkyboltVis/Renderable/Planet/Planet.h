/* Copyright 2012-2020 Matthew Reid
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
#include "SkyboltVis/Renderable/Clouds/VolumeClouds.h"
#include "SkyboltVis/Renderable/Planet/Features/PlanetFeaturesSource.h"
#include "SkyboltVis/Renderable/Forest/GpuForest.h"

#include <SkyboltSim/SkyboltSimFwd.h>
#include <SkyboltSim/Spatial/LatLon.h>
#include <SkyboltCommon/File/FileLocator.h>

#include <osg/MatrixTransform>
#include <osg/Uniform>
#include <osg/Vec3f>

#include <boost/optional.hpp>

namespace skybolt {
namespace vis {

struct PlanetConfig
{
	px_sched::Scheduler* scheduler;
	const ShaderPrograms* programs;
	Scene* scene;
	BuildingTypesPtr buildingTypes; //!< optional
	sim::LatLon latLonOrigin;
	float innerRadius;
	PlanetTileSources planetTileSources;
	VisFactoryRegistry* visFactoryRegistry;
	bool waterEnabled = true;
	osg::ref_ptr<osg::Texture2D> cloudsTexture; //!< Set to null to disable clouds
	boost::optional<BruentonAtmosphereConfig> atmosphereConfig;
	file::FileLocator fileLocator;
	std::vector<file::Path> featureTreeFiles;
	std::string featureTilesDirectoryRelAssetPackage;
	std::optional<ForestParams> forestParams;
	DetailMappingTechniquePtr detailMappingTechnique;
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

	PlanetSurface* getSurface() const { return mPlanetSurface.get(); } //!< Never returns null

	PlanetFeatures* getPlanetFeatures() const { return mPlanetFeatures.get(); } //!< May return null

	osg::ref_ptr<BruentonAtmosphere> getAtmosphere() const; //!< May return null

	osg::ref_ptr<WaterMaterial> getWaterMaterial() const; //!< May return null

	void setJulianDate(double date)
	{
		mJulianDate = date;
	}

	void setCloudsVisible(bool visible);
	bool getCloudsVisible(void) const { return mCloudsVisible; }

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
	osg::ref_ptr<WaterMaterial> mWaterMaterial;
	OceanPtr mOcean;
	std::unique_ptr <ReflectionCameraController> mReflectionCameraController;
	
	std::unique_ptr<PlanetSurface> mPlanetSurface;
	std::shared_ptr<PlanetSky> mPlanetSky;
	std::unique_ptr<PlanetFeatures> mPlanetFeatures; //!< Can be null
	VolumeCloudsPtr mVolumeClouds;
	osg::ref_ptr<BruentonAtmosphere> mAtmosphere;

	osg::Uniform* mPlanetCenterUniform;
	osg::Uniform* mPlanetMatrixInvUniform;

	double mInnerRadius;
	boost::optional<float> mAtmosphereScaleHeight;
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
